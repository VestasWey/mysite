#pragma once

#include <map>
#include <memory>

#include "url_canon.h"

namespace
{
    class URLRequestImpl;
}

namespace mctm
{
    class HttpResponseHeaders
    {
    public:
        long response_code() const { return response_code_; }
        const std::string& response_header() { return response_header_; }

        void Reset()
        {
            response_code_ = 0;
            response_header_.clear();
        }

    private:
        friend class URLRequestImpl;

        long response_code_ = 0;
        std::string response_header_;
    };

    class HttpResponseInfo
    {
    public:
        const HttpResponseHeaders* response_headers() const { return &response_headers_; }

        void Reset()
        {
            response_headers_.Reset();
        }

    private:
        friend class URLRequestImpl;

        HttpResponseHeaders response_headers_;
    };

    class URLRequestStatus
    {
    public:
        enum class Status
        {
            // Request succeeded, |error_| will be 0.
            SUCCESS = 0,

            // An IO request is pending, and the caller will be informed when it is
            // completed.
            IO_PENDING,

            // Request was cancelled programatically.
            CANCELED,

            // The request failed for some reason. |error_| may have more information.
            FAILED,
        };

        URLRequestStatus() : status_(Status::SUCCESS), error_(0) {}
        URLRequestStatus(Status s, int e) : status_(s), error_(e) {}

        Status status() const { return status_; }
        void set_status(Status s) { status_ = s; }

        int error() const { return error_; }
        void set_error(int e) { error_ = e; }

        // Returns true if the status is success, which makes some calling code more
        // convenient because this is the most common test.
        bool is_success() const
        {
            return status_ == Status::SUCCESS || status_ == Status::IO_PENDING;
        }

        // Returns true if the request is waiting for IO.
        bool is_io_pending() const
        {
            return status_ == Status::IO_PENDING;
        }

    private:
        // Application level status.
        Status status_ = Status::SUCCESS;

        // Error code from the network layer if an error was encountered.
        int error_ = 0;
    };

    class URLRequestContext;
    class URLRequest
    {
    public:
        class Delegate : public std::enable_shared_from_this<Delegate>
        {
        public:
            virtual void OnRequestStarted() {}
            virtual void OnRequestFailed(const char* err_msg) {}
            virtual void OnRequestCompleted() {}
            virtual void OnRequestProgress(double dltotal, double dlnow, double ultotal, double ulnow) {}
            virtual void OnReponseDataRecv(const char *ptr, size_t size) = 0;

        protected:
            virtual ~Delegate() = default;
        };

        URLRequest(const CanonURL& url,
            std::weak_ptr<Delegate> delegate,
            const URLRequestContext* context);
        virtual ~URLRequest();

        bool Start();
        void Cancel();
        bool Restart();

        bool is_pending() const { return status_.is_io_pending(); }
        const CanonURL& url() const { return url_; }
        const std::string& method() const { return method_; }
        void set_method(const std::string& method);
        void set_header(const std::map<std::string, std::string>& request_headers);
        void set_header(const std::string& key, const std::string& value);
        void set_upload(const std::string& upload_data);

        const HttpResponseInfo* response_info() const;

    private:
        void PrepareToRestart();
        // invoke by URLRequestImpl
        void OnRequestStarted();
        void OnRequestFailed(int err_code, const char* err_msg);
        void OnRequestCompleted(const HttpResponseInfo* rsp_info);
        void OnRequestProgress(double dltotal, double dlnow, double ultotal, double ulnow);
        void OnReponseDataRecv(const char *ptr, size_t size);

    private:
        friend class URLRequestImpl;

        CanonURL url_;
        std::weak_ptr<Delegate> delegate_;
        const URLRequestContext* context_ = nullptr;
        std::unique_ptr<URLRequestImpl> request_;
        std::string method_;  // "GET", "POST", etc. Should be all uppercase.
        std::string upload_data_stream_;
        URLRequestStatus status_;
    };
}