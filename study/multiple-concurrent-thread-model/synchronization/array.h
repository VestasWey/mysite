#pragma once

#include <vector>
#include <atomic>
#include <mutex>
#include <algorithm>

namespace mctm
{
    // 循环数组
    // 
    // 多个生产者的情况下，写还是应该用同步锁，不能用无锁方式，否则还是会发生同时写的情况；
    // 因为无锁情况下通过CAS等无锁操作只能原子的得到要操作的内存地址区，如果不是循环数组，那么这样没问题，
    // 但这里是循环数组，会出现两个write操作的内存区域重叠的情况，此时还是会发送写冲突；
    // 
    // 多个消费者可以不加锁，消费者应该要接受循环数组读的内存区有重叠也没关系
    template<class T>
    class CycleArray
    {
    public:
        explicit CycleArray(size_t capacity)
            : capacity_(capacity),
            vector_(capacity_)
        {
        }

        void write(const T* elem, size_t len)
        {
            if (len <= 0)
            {
                return;
            }

            // 要写的数据超过环长度的话则只写输入数据的最后那一环的长度，否则会产生一个write调用却发生多次数据覆写的情况
            if (len > capacity_)
            {
                elem = elem + len - capacity_;
                len = capacity_;
            }

            std::lock_guard<std::mutex> lock(vct_mutex_);

            unsigned long long wc;
            do 
            {
                wc = write_total_;
            } while (!write_total_.compare_exchange_strong(wc, wc + len));

            size_t woft = wc % capacity_;    // 得到初始写偏移

            // 根据len计算需要操作的内存段
            wc = len;
            while (wc > 0)
            {
                unsigned long long cc = capacity_ - woft;
                cc = std::min(cc, wc);

                for (size_t i = 0; i < cc; ++i)
                {
                    vector_[i + woft] = *(elem + i);
                }

                wc -= cc;

                woft += (size_t)cc;
                if (woft >= capacity_)
                {
                    woft = 0;
                }
            }
        }

        T read()
        {
            unsigned long long rc;
            do
            {
                rc = read_total_;
            } while (!read_total_.compare_exchange_strong(rc, rc + 1));

            size_t oft = rc % capacity_;    // 得到初始读偏移

            return vector_[oft];
        }

        std::vector<T> read(size_t len)
        {
            if (len <= 0)
            {
                return {};
            }

            std::vector<T> vct;
            vct.resize(len);
            unsigned long long oc;
            do
            {
                oc = read_total_;
            } while (!read_total_.compare_exchange_strong(oc, oc + len));

            size_t oft = oc % capacity_;    // 得到初始读偏移

            // 根据len计算需要操作的内存段
            oc = len;
            size_t idx = 0;
            while (oc > 0)
            {
                unsigned long long cc = capacity_ - oft;
                cc = std::min(cc, oc);

                for (size_t i = 0; i < cc; ++i)
                {
                    vct[idx + i] = vector_[oft + i];
                }

                oc -= cc;
                idx += cc;
                oft += (size_t)cc;
                if (oft >= capacity_)
                {
                    oft = 0;
                }
            }

            return vct;
        }

    private:
        unsigned long capacity_ = 0;
        std::atomic_ullong read_total_ = 0;
        std::atomic_ullong write_total_ = 0;    // 只会正向增长的写数据数，每次写都先对capacity_求模得到数组的写偏移
        std::vector<T> vector_;
        std::mutex vct_mutex_;
    };


    //template<class T>
    //class Array
    //{
    //    explicit CycleArray(size_t capacity)
    //        : capacity_(capacity),
    //        vector_(capacity_)
    //    {
    //    }

    //    bool push_back(const T& elem)
    //    {
    //        size_t cs = size_;

    //        do 
    //        {
    //            cs = size_;
    //            if (cs >= capacity_)
    //            {
    //                // the queue is full
    //                return false;
    //            }
    //        } while (!size_.compare_exchange_strong(cs, cs + 1));

    //        cs = cs + 1;
    //        vector_[cs] = elem;
    //    }

    //    void pop()
    //    {
    //        size_t cs = size_;

    //        do
    //        {
    //            cs = size_;
    //            if (cs <= 0)
    //            {
    //                // the queue is empty
    //                return;
    //            }
    //        } while (!size_.compare_exchange_strong(cs, cs - 1));

    //        cs = cs - 1;
    //        vector_[cs] = elem;
    //    }

    //    T at(size_t index)
    //    {
    //        if (index >= size_)
    //        {
    //            throw std::exception("invalid param");
    //        }
    //    }

    //private:
    //    std::atomic_uint size_ = 0;
    //    size_t capacity_ = 0;
    //    std::vector<T> vector_;
    //};
}


void TestCycleArray();