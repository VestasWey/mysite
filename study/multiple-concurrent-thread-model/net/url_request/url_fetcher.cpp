#include "url_fetcher.h"

#include "http_request_headers.h"
#include "logging/logging.h"
#include "message_loop/message_loop.h"


namespace mctm
{
    std::shared_ptr<URLFetcher> URLFetcher::Create(
        const CanonURL& url, RequestType request_type, URLFetcherDelegate* delegate)
    {
        std::shared_ptr<URLFetcher> ptr(new URLFetcher(url, request_type, delegate),
            URLFetcher::Deleter);
        return ptr;
    }

    void URLFetcher::Deleter(URLFetcher* fetcher)
    {
        delete fetcher;
    }

    URLFetcher::URLFetcher(const CanonURL& url, URLFetcher::RequestType request_type, URLFetcherDelegate* delegate)
        : url_(url)
        , request_type_(request_type)
        , delegate_(delegate)
    {
    }

    URLFetcher::~URLFetcher()
    {
        delegate_ = nullptr;
        Stop();
    }

    void URLFetcher::SetRequestContext(URLRequestContext* request_context)
    {
        DCHECK(request_context);
        request_context_ = request_context;
    }

    void URLFetcher::SetNetworkTaskRunner(SingleThreadTaskRunner network_task_runner)
    {
        DCHECK(!network_task_runner_);
        DCHECK(network_task_runner);
        network_task_runner_ = network_task_runner;
    }

    bool URLFetcher::Start()
    {
        DCHECK(network_task_runner_);
        if (!network_task_runner_)
        {
            return false;
        }

        DCHECK(MessageLoop::current());
        if (!MessageLoop::current())
        {
            return false;
        }
        
        started_ = true;
        delegate_task_runner_ = MessageLoop::current()->shared_from_this();
        network_task_runner_->PostTask(FROM_HERE, Bind(&URLFetcher::StartOnIOThread, GetWeakPtr()));
        return true;
    }

    void URLFetcher::Stop()
    {
        started_ = false;

        if (request_)
        {
            request_->Cancel();

            DCHECK(network_task_runner_);
            if (network_task_runner_)
            {
                network_task_runner_->DeleteSoon(FROM_HERE, request_.release());
            }
        }
    }

    void URLFetcher::SetUploadData(const std::string& upload_content_type, const std::string& upload_content)
    {
        DCHECK(!is_chunked_upload_);
        DCHECK(!upload_content_set_);
        DCHECK(upload_content_.empty());
        DCHECK(upload_file_path_.empty());
        DCHECK(upload_content_type_.empty());

        // Empty |upload_content_type| is allowed iff the |upload_content| is empty.
        DCHECK(upload_content.empty() || !upload_content_type.empty());

        upload_content_type_ = upload_content_type;
        upload_content_ = upload_content;
        upload_content_set_ = true;
    }

    void URLFetcher::SetUploadFilePath(const std::string& upload_content_type, 
        const std::wstring& file_path, unsigned __int64 range_offset, unsigned __int64 range_length,
        SingleThreadTaskRunner file_task_runner)
    {
        DCHECK(!is_chunked_upload_);
        DCHECK(!upload_content_set_);
        DCHECK(upload_content_.empty());
        DCHECK(upload_file_path_.empty());
        //DCHECK_EQ(upload_range_offset_, 0);
        //DCHECK_EQ(upload_range_length_, 0);
        DCHECK(upload_content_type_.empty());
        DCHECK(!upload_content_type.empty());

        upload_content_type_ = upload_content_type;
        upload_file_path_ = file_path;
        upload_range_offset_ = range_offset;
        upload_range_length_ = range_length;
        upload_file_task_runner_ = file_task_runner;
        upload_content_set_ = true;
    }

    const HttpResponseHeaders* URLFetcher::GetResponseHeaders() const
    {
        if (request_ && !request_->is_pending())
        {
            return request_->response_info()->response_headers();
        }
        return nullptr;
    }

    void URLFetcher::StartOnIOThread()
    {
        if (!started_)
        {
            return;
        }

        DCHECK(request_context_);
        DCHECK(!request_);

        request_ = request_context_->CreateURLRequest(url_, weak_from_this());

        switch (request_type_)
        {
        case mctm::URLFetcher::GET:
            request_->set_method("GET");
            break;
        case mctm::URLFetcher::POST:
            request_->set_method("POST");
            break;
        case mctm::URLFetcher::HEAD:
            request_->set_method("HEAD");
            break;
        case mctm::URLFetcher::DELETE_REQUEST:
            request_->set_method("DELETE");
            break;
        case mctm::URLFetcher::PUT:
            request_->set_method("PUT");
            break;
        case mctm::URLFetcher::PATCH:
            request_->set_method("PATCH");
            break;
        default:
            break;
        }
        request_->set_upload(upload_content_);

        if (!upload_content_type_.empty())
        {
            request_->set_header(HttpRequestHeaders::kContentType, upload_content_type_);
        }

        if (started_)
        {
            request_->Start();
        }
    }

    std::weak_ptr<URLFetcher> URLFetcher::GetWeakPtr()
    {
        return std::weak_ptr<URLFetcher>(std::dynamic_pointer_cast<URLFetcher>(shared_from_this()));
    }

    void URLFetcher::SetURLFetcherDelegate(URLFetcherDelegate* delegate)
    {
        delegate_ = delegate;
    }

    // URLRequest::Delegate£¬invoke on io thread
    void URLFetcher::OnRequestStarted()
    {
        if (delegate_ && delegate_task_runner_)
        {
            delegate_task_runner_->PostTask(FROM_HERE, 
                Bind(&URLFetcher::InformDelegateRequestStarted, GetWeakPtr()));
        }
    }

    void URLFetcher::OnRequestFailed(const char* err_msg)
    {
        if (delegate_ && delegate_task_runner_)
        {
            delegate_task_runner_->PostTask(FROM_HERE,
                Bind(&URLFetcher::InformDelegateRequestFailed, GetWeakPtr(),
                    std::string(err_msg)));
        }
    }

    void URLFetcher::OnRequestCompleted()
    {
        if (delegate_ && delegate_task_runner_)
        {
            delegate_task_runner_->PostTask(FROM_HERE,
                Bind(&URLFetcher::InformDelegateRequestCompleted, GetWeakPtr()));
        }
    }

    void URLFetcher::OnRequestProgress(double dltotal, double dlnow, double ultotal, double ulnow)
    {
        if (delegate_ && delegate_task_runner_)
        {
            delegate_task_runner_->PostTask(FROM_HERE,
                Bind(&URLFetcher::InformDelegateRequestProgress, GetWeakPtr(),
                    dltotal, dlnow, ultotal, ulnow));
        }
    }

    void URLFetcher::OnReponseDataRecv(const char *ptr, size_t size)
    {
        if (delegate_ && delegate_task_runner_)
        {
            std::shared_ptr<std::string> recv_data = std::make_shared<std::string>(ptr, size);
            delegate_task_runner_->PostTask(FROM_HERE, 
                Bind(&URLFetcher::InformDelegateReponseDataRecv, GetWeakPtr(),
                    recv_data));
        }
    }

    // invoke on caller thread
    void URLFetcher::InformDelegateRequestStarted()
    {
        if (delegate_)
        {
            delegate_->OnURLFetchStart(this);
        }
    }

    void URLFetcher::InformDelegateRequestFailed(const std::string& err_msg)
    {
        if (delegate_)
        {
            delegate_->OnURLFetchFailed(this);
        }
    }

    void URLFetcher::InformDelegateRequestCompleted()
    {
        if (delegate_)
        {
            delegate_->OnURLFetchComplete(this);
        }
    }

    void URLFetcher::InformDelegateRequestProgress(double dltotal, double dlnow, double ultotal, double ulnow)
    {
        if (delegate_)
        {
            delegate_->OnURLFetchDownloadProgress(this, dlnow, dltotal);

            delegate_->OnURLFetchUploadProgress(this, ulnow, ultotal);
        }
    }

    void URLFetcher::InformDelegateReponseDataRecv(std::shared_ptr<std::string>& download_data)
    {
        if (delegate_ && download_data)
        {
            delegate_->OnURLFetchDownloadData(this, download_data->data(), download_data->length());
        }
    }

}