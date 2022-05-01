#pragma once

#include <atomic>
#include <memory>

#include "data_encapsulation/smart_pointer.h"
#include "net/url_request/url_request_context.h"

namespace mctm
{
    class URLFetcher;
    class URLFetcherDelegate
    {
    public:
        // This will be called when the URL has been fetched, successfully or not.
        // Use accessor methods on |source| to get the results.
        virtual void OnURLFetchStart(const URLFetcher* source) {}
        virtual void OnURLFetchFailed(const URLFetcher* source) {}
        virtual void OnURLFetchComplete(const URLFetcher* source) {}

        // This will be called when some part of the response is read. |current|
        // denotes the number of bytes received up to the call, and |total| is the
        // expected total size of the response (or -1 if not determined).
        virtual void OnURLFetchDownloadProgress(const URLFetcher* source,
            double current, double total)
        {
        }

        // This will be called when some part of the response is read.
        // |download_data| contains the current bytes received since the last call.
        // This will be called after ShouldSendDownloadData() and only if the latter
        // returns true.
        virtual void OnURLFetchDownloadData(const URLFetcher* source,
            const char *ptr, size_t size) = 0;

        // This will be called when uploading of POST or PUT requests proceeded.
        // |current| denotes the number of bytes sent so far, and |total| is the
        // total size of uploading data (or -1 if chunked upload is enabled).
        virtual void OnURLFetchUploadProgress(const URLFetcher* source,
            double current, double total)
        {
        }

    protected:
        virtual ~URLFetcherDelegate() = default;
    };

    class URLRequest;
    class URLFetcher 
        : public URLRequest::Delegate
    {
        static void Deleter(URLFetcher* fetcher);
    public:
        enum RequestType
        {
            GET,
            POST,
            HEAD,
            DELETE_REQUEST,   // DELETE is already taken on Windows.
                              // <winnt.h> defines a DELETE macro.
            PUT,
            PATCH,
        };

        static std::shared_ptr<URLFetcher> Create(const CanonURL& url,
            RequestType request_type,
            URLFetcherDelegate* delegate);

        std::weak_ptr<URLFetcher> GetWeakPtr();

        void SetURLFetcherDelegate(URLFetcherDelegate* delegate);
        void SetRequestContext(URLRequestContext* request_context);
        void SetNetworkTaskRunner(SingleThreadTaskRunner network_task_runner);

        void SetUploadData(const std::string& upload_content_type,
            const std::string& upload_content);
        void SetUploadFilePath(const std::string& upload_content_type,
            const std::wstring& file_path,
            unsigned __int64 range_offset,
            unsigned __int64 range_length,
            SingleThreadTaskRunner file_task_runner);

        bool Start();
        void Stop();

        const HttpResponseHeaders* GetResponseHeaders() const;

    protected:
        // URLRequest::Delegate
        void OnRequestStarted() override;
        void OnRequestFailed(const char* err_msg) override;
        void OnRequestCompleted() override;
        void OnRequestProgress(double dltotal, double dlnow, double ultotal, double ulnow) override;
        void OnReponseDataRecv(const char *ptr, size_t size) override;

        void InformDelegateRequestStarted();
        void InformDelegateRequestFailed(const std::string& err_msg);
        void InformDelegateRequestCompleted();
        void InformDelegateRequestProgress(double dltotal, double dlnow, double ultotal, double ulnow);
        void InformDelegateReponseDataRecv(std::shared_ptr<std::string>& download_data);

    private:
        URLFetcher(const CanonURL& url,
            RequestType request_type,
            URLFetcherDelegate* delegate);
        virtual ~URLFetcher();

        void StartOnIOThread();

    private:
        friend class std::shared_ptr<URLFetcher>;

        CanonURL url_;
        RequestType request_type_ = RequestType::GET;
        URLFetcherDelegate* delegate_ = nullptr;
        URLRequestContext* request_context_;

        std::atomic_bool started_ = false;
        std::unique_ptr<URLRequest> request_;   // The actual request this wraps

        bool is_chunked_upload_ = false;           // True if using chunked transfer encoding

        bool upload_content_set_ = false;   // SetUploadData has been called
        std::string upload_content_type_;   // MIME type of POST payload
        std::string upload_content_;        // HTTP POST payload

        std::wstring upload_file_path_;  // Path to file containing POST payload
        unsigned __int64 upload_range_offset_ = 0;       // Offset from the beginning of the file to be uploaded.
        unsigned __int64 upload_range_length_ = 0;       // The length of the part of file to be uploaded.

        SingleThreadTaskRunner delegate_task_runner_;
        SingleThreadTaskRunner network_task_runner_;
        //SingleThreadTaskRunner file_task_runner_;
        SingleThreadTaskRunner upload_file_task_runner_;
    };
}