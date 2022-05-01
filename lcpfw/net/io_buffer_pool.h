#pragma once

#include <memory>
#include <mutex>
#include <queue>

#include "data_encapsulation/smart_pointer.h"

namespace mctm
{
    static const size_t kIOBufferSize = 4096;

    enum class AsyncType
    {
        Unknown,

        // PIPE
        Pipe_Accept,
        Pipe_Read,
        Pipe_Write,
    };

    class IOBuffer
    {
    public:
        IOBuffer()
        {
            Reset();
        }

        void Reset()
        {
            memset(buffer, 0, kIOBufferSize);
        }

        char buffer[kIOBufferSize];
        int len = kIOBufferSize;
    };

    class IOBufferPool
    {
        using BufferQueue = std::queue<IOBufferRef>;

    public:
        IOBufferPool(unsigned int init_count, unsigned int max_count)
            : max_count_(max_count)
        {
            init_count = (init_count < max_count) ? init_count : max_count;
            for (unsigned int i = 0; i < init_count; ++i)
            {
                idle_queue_.push(std::make_shared<IOBuffer>());
            }
        }

        IOBufferRef GetIOBuffer()
        {
            IOBufferRef buffer;

            {
                std::lock_guard<std::mutex> lock(idle_lock_);
                if (!idle_queue_.empty())
                {
                    buffer = idle_queue_.front();
                    idle_queue_.pop();
                }
            }

            if (!buffer)
            {
                buffer = std::make_unique<IOBuffer>();
            }

            return buffer;
        }

        void ReleaseIOBuffer(IOBufferRef buffer)
        {
            {
                std::lock_guard<std::mutex> lock(outstanding_lock_);
                if (outstanding_queue_.size() < max_count_)
                {
                    outstanding_queue_.push(buffer);
                    return;
                }
            }

            if (buffer)
            {
                buffer.reset();
            }
        }

    private:
        const unsigned int max_count_ = 0;

        std::mutex outstanding_lock_;
        BufferQueue outstanding_queue_;

        std::mutex idle_lock_;
        BufferQueue idle_queue_;
    };

}
