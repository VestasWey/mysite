#include "url_request.h"

#include <algorithm>
#include <atomic>
#include <tuple>

#include "logging/logging.h"
#include "third_party/libcurl/include/curl/curl.h"

#if LIBCURL_VERSION_NUM >= 0x073d00
/* In libcurl 7.61.0, support was added for extracting the time in plain
   microseconds. Older libcurl versions are stuck in using 'double' for this
   information so we complicate this example a bit by supporting either
   approach. */
#define TIME_IN_US 1  
#define TIMETYPE curl_off_t
#define TIMEOPT CURLINFO_TOTAL_TIME_T
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3000000
#else
#define TIMETYPE double
#define TIMEOPT CURLINFO_TOTAL_TIME
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3
#endif

namespace
{
    class CURLInitializer
    {
    public:
        CURLInitializer()
        {
            CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
            DCHECK(CURLE_OK == ret);
        }

        ~CURLInitializer()
        {
            curl_global_cleanup();
        }
    };
    CURLInitializer curl_init_obj;

    class URLRequestImpl
    {
    public:
        explicit URLRequestImpl(mctm::URLRequest* request)
            : request_(request)
        {
            Init();
        }

        virtual ~URLRequestImpl()
        {
            Uninit();
        }

        bool SetRequestTimeout(long timeout_in_ms)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout_in_ms));
        }

        bool SetConnectTimeout(long timeout_in_ms)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, timeout_in_ms));
        }

        bool SetVerbose(bool onoff)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_VERBOSE, onoff ? 1 : 0));
        }

        bool SetRedirect(bool enable)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, enable ? 1 : 0));
        }

        bool SetUrl(const char* url)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_URL, url));
        }

        bool SetHeader(const char* header_item)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            header_ = curl_slist_append(header_, header_item);
            return !!header_;
        }

        bool SetData(const std::string* data)
        {
            if (do_request_now_)
            {
                return false;
            }

            std::get<0>(request_data_) = data;
            std::get<1>(request_data_) = 0;
            return true;
        }

        bool DoRequest()
        {
            if (curl_ && !do_request_now_)
            {
                CURLcode ret = CURLE_OK;
                stop_ = false;

                // callback
                request_->OnRequestStarted();

                if (header_)
                {
                    ret = curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_);
                }

                if (CURLE_OK == ret)
                {
                    do_request_now_ = true;
                    response_info_.Reset();
                    ret = curl_easy_perform(curl_);
                    do_request_now_ = false;

                    if (CURLE_OK == ret)
                    {
                        ret = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, 
                            &response_info_.response_headers_.response_code_);
                    }
                }

                if (CURLE_OK == ret)
                {
                    // callback
                    request_->OnRequestCompleted(&response_info_);
                }
                else
                {
                    const char * err_msg = curl_easy_strerror(ret);
                    // callback
                    request_->OnRequestFailed(ret, err_msg);
                }

                return (CURLE_OK == ret);
            }
            return false;
        }

        void Stop()
        {
            if (do_request_now_)
            {
                stop_ = true;
            }
        }

        bool GetResponseHeaderSize(long* header_size)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            if (!header_size)
            {
                return false;
            }

            *header_size = -1;
            return (CURLE_OK == curl_easy_getinfo(curl_, CURLINFO_HEADER_SIZE, header_size));
        }

        const mctm::HttpResponseInfo* response_info() const { return &response_info_; }

    private:
        bool Init()
        {
            if (!curl_)
            {
                curl_ = curl_easy_init();
                if (curl_)
                {
                    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "libcurl-agent/1.0");

                    curl_easy_setopt(curl_, CURLOPT_READFUNCTION, OnRequestSendDataFunction);
                    curl_easy_setopt(curl_, CURLOPT_READDATA, this);

                    curl_easy_setopt(curl_, CURLOPT_HEADER, 0);
                    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, OnResponseHeaderFunction);
                    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, this);

                    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, OnResponseRecvDataFunction);
                    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);

                    curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 0);
#if LIBCURL_VERSION_NUM >= 0x072000
                    curl_easy_setopt(curl_, CURLOPT_XFERINFOFUNCTION, OnProgressFunction);
                    curl_easy_setopt(curl_, CURLOPT_XFERINFODATA, this);
#else
                    curl_easy_setopt(curl_, CURLOPT_PROGRESSFUNCTION, OnProgressFunction);
                    curl_easy_setopt(curl_, CURLOPT_PROGRESSDATA, this);
#endif
                }

                return !!curl_;
            }
            return false;
        }

        void Uninit()
        {
            DCHECK(!do_request_now_);;

            if (header_)
            {
                curl_slist_free_all(header_);
                header_ = nullptr;
            }
            if (curl_)
            {
                curl_easy_cleanup(curl_);
                curl_ = nullptr;
            }

            std::tuple<const std::string*, size_t> ept{};
            request_data_.swap(ept);
            response_info_.Reset();
        }

        // static callback
        static size_t OnRequestSendDataFunction(char *buffer, size_t size, size_t nitems, void *userdata)
        {
            URLRequestImpl* impl = reinterpret_cast<URLRequestImpl*>(userdata);

            if (impl->stop_)
            {
                return CURL_READFUNC_ABORT;
            }

            size_t at_most = size * nitems;
            size_t total = std::min(at_most, std::get<0>(impl->request_data_)->length() - std::get<1>(impl->request_data_));

            std::copy_n(std::get<0>(impl->request_data_)->data() + std::get<1>(impl->request_data_), total, buffer);
            std::get<1>(impl->request_data_) += total;

            return total;
        }

        static size_t OnResponseHeaderFunction(char *buffer, size_t size, size_t nitems, void *userdata)
        {
            URLRequestImpl* impl = reinterpret_cast<URLRequestImpl*>(userdata);

            size_t total = size * nitems;
            impl->response_info_.response_headers_.response_header_.append(reinterpret_cast<const char*>(buffer), total);

            return total;
        }

        static size_t OnResponseRecvDataFunction(char *ptr, size_t size, size_t nmemb, void *userdata)
        {
            URLRequestImpl* impl = reinterpret_cast<URLRequestImpl*>(userdata);

            if (impl->stop_)
            {
                return 0; //CURL_WRITEFUNC_PAUSE
            }

            size_t total = size * nmemb;
            //impl->response_info_.response_data_.append(ptr, total);

            impl->request_->OnReponseDataRecv((const char*)ptr, total);

            return total;
        }

        static int OnProgressFunction(void *userdata, double dltotal, double dlnow, double ultotal, double ulnow)
        {
            URLRequestImpl* impl = reinterpret_cast<URLRequestImpl*>(userdata);

            if (impl->stop_)
            {
                return -1;
            }

            impl->request_->OnRequestProgress(dltotal, dlnow, ultotal, ulnow);

            // download: dltotal¡¢dlnow
            // upload: ultotal¡¢ulnow
            //TIMETYPE curtime = 0;
            //curl_easy_getinfo(curl, TIMEOPT, &curtime);

            return 0;
        }

    private:
        mctm::URLRequest* request_ = nullptr;
        CURL *curl_ = nullptr;
        curl_slist* header_ = nullptr;
        std::tuple<const std::string*, size_t> request_data_;

        mctm::HttpResponseInfo response_info_;

        std::atomic_bool do_request_now_ = false;
        std::atomic_bool stop_ = false;
    };
}

namespace mctm
{
    URLRequest::URLRequest(const CanonURL& url, std::weak_ptr<Delegate> delegate, const URLRequestContext* context)
        : url_(url)
        , delegate_(delegate)
        , context_(context)
    {
        request_ = std::make_unique<URLRequestImpl>(this);
        request_->SetUrl(url_.spec().c_str());
    }

    URLRequest::~URLRequest()
    {
    }

    bool URLRequest::Start()
    {
        if (status_.is_io_pending() || 
            status_.status() == URLRequestStatus::Status::CANCELED)
        {
            return false;
        }

        status_.set_status(URLRequestStatus::Status::IO_PENDING);
        return request_->DoRequest();
    }

    void URLRequest::Cancel()
    {
        if (status_.is_io_pending())
        {
            request_->Stop();
        }

        status_.set_status(URLRequestStatus::Status::CANCELED);
    }

    bool URLRequest::Restart()
    {
        if (!status_.is_io_pending())
        {
            PrepareToRestart();
            return Start();
        }
        return false;
    }

    void URLRequest::set_method(const std::string& method)
    {
        if (!status_.is_io_pending())
        {
            method_ = method;
        }
    }

    void URLRequest::set_header(const std::map<std::string, std::string>& request_headers)
    {
        if (!status_.is_io_pending())
        {
            for (auto& iter : request_headers)
            {
                set_header(iter.first, iter.second);
            }
        }
    }

    void URLRequest::set_header(const std::string& key, const std::string& value)
    {
        if (!status_.is_io_pending())
        {
            std::string header_item;
            header_item.append(key);
            header_item.append(": ");
            header_item.append(value);
            request_->SetHeader(header_item.c_str());
        }
    }

    void URLRequest::set_upload(const std::string& upload_data)
    {
        if (!status_.is_io_pending())
        {
            upload_data_stream_ = upload_data;

            request_->SetData(&upload_data_stream_);
        }
    }

    const HttpResponseInfo* URLRequest::response_info() const
    {
        return request_->response_info();
    }

    void URLRequest::PrepareToRestart()
    {
        status_.set_status(URLRequestStatus::Status::SUCCESS);
    }

    // invoke by URLRequestImpl
    void URLRequest::OnRequestStarted()
    {
        auto s_ptr = delegate_.lock();
        if (s_ptr)
        {
            s_ptr->OnRequestStarted();
        }
    }

    void URLRequest::OnRequestFailed(int err_code, const char* err_msg)
    {
        DLOG(WARNING) << "url request failed, code = " << err_code << ", msg = " << err_msg;

        status_.set_status(URLRequestStatus::Status::FAILED);

        auto s_ptr = delegate_.lock();
        if (s_ptr)
        {
            s_ptr->OnRequestFailed(err_msg);
        }
    }

    void URLRequest::OnRequestCompleted(const HttpResponseInfo* rsp_info)
    {
        DLOG_IF(WARNING, rsp_info->response_headers()->response_code() != 200) <<
            "http request failed, http_status = " << rsp_info->response_headers()->response_code();

        status_.set_status(URLRequestStatus::Status::SUCCESS);

        auto s_ptr = delegate_.lock();
        if (s_ptr)
        {
            s_ptr->OnRequestCompleted();
        }
    }

    void URLRequest::OnRequestProgress(double dltotal, double dlnow, double ultotal, double ulnow)
    {
        auto s_ptr = delegate_.lock();
        if (s_ptr)
        {
            s_ptr->OnRequestProgress(dltotal, dlnow, ultotal, ulnow);
        }
    }

    void URLRequest::OnReponseDataRecv(const char *ptr, size_t size)
    {
        auto s_ptr = delegate_.lock();
        if (s_ptr)
        {
            s_ptr->OnReponseDataRecv(ptr, size);
        }
    }

}