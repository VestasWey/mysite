#include "stdafx.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <iostream>           // std::cout

namespace
{
    template<class _Type>
    void PrintUnorderedInfo(const _Type& umap)
    {
        std::cout << "bucket_count=" << umap.bucket_count() << ", "
            << "bucket_size=" << umap.bucket_size(0) << ", "
            << "load_factor=" << umap.load_factor() << ", "
            << "max_load_factor=" << umap.max_load_factor() << ", "
            << "size=" << umap.size() << ", ";
        std::cout << std::endl;
    }

    void unordered_map_example()
    {
        std::unordered_map<int, int> umap;

        PrintUnorderedInfo(umap);

        umap.max_load_factor(3);

        int i = 0;
        for (int x = 0; x < 8; x++, i++)
        {
            umap.emplace(i * 2, i);
        }
        PrintUnorderedInfo(umap);

        auto iter = umap.find(5);
        std::cout << "enum key=5 to end" << std::endl;
        for (auto tmp = iter; tmp != umap.end(); ++tmp)
        {
            std::cout << "bucket=" << umap.bucket(tmp->first) << ", \tk=" << tmp->first << ", \tv=" << tmp->second << std::endl;
        }

        std::cout << "enum equal_range" << std::endl;
        auto range = umap.equal_range(2);
        for (auto tmp = range.first; tmp != range.second; ++tmp)
        {
            std::cout << "bucket=" << umap.bucket(tmp->first) << ", \tk=" << tmp->first << ", \tv=" << tmp->second << std::endl;
        }

        // insert invoke rehash
        for (int x = 0; x < 10; x++, i++)
        {
            umap.emplace(i * 2, i);
        }

        // iter valid
        std::cout << "after first emplace, rehash£¬enum key=5 to end" << std::endl;
        for (auto tmp = iter; tmp != umap.end(); ++tmp)
        {
            std::cout << "bucket=" << umap.bucket(tmp->first) << ", \tk=" << tmp->first << ", \tv=" << tmp->second << std::endl;
        }

        PrintUnorderedInfo(umap);

        std::cout << "emplace to 64 elems" << std::endl;
        for (int x = 0; x < 46; x++, i++)
        {
            umap.emplace(i, i);
        }
        PrintUnorderedInfo(umap);
        for (auto& tmp : umap)
        {
            std::cout << "bucket=" << umap.bucket(tmp.first) << ", \tk=" << tmp.first << ", \tv=" << tmp.second << std::endl;
        }

        //
        std::cout << "rehash to 8" << std::endl;
        umap.rehash(8);
        PrintUnorderedInfo(umap);
        for (auto& tmp : umap)
        {
            std::cout << "bucket=" << umap.bucket(tmp.first) << ", \tk=" << tmp.first << ", \tv=" << tmp.second << std::endl;
        }

        // 
        std::cout << "reserve to 28" << std::endl;
        umap.reserve(28);
        PrintUnorderedInfo(umap);
        for (auto& tmp : umap)
        {
            std::cout << "bucket=" << umap.bucket(tmp.first) << ", \tk=" << tmp.first << ", \tv=" << tmp.second << std::endl;
        }

        // erase
        std::cout << "foreach erase" << std::endl;
        for (int i = 0; i < 64; i++)
        {
            umap.erase(i);
            PrintUnorderedInfo(umap);
        }
    }

    void unordered_set_example()
    {
        std::unordered_set<int> uset;

        PrintUnorderedInfo(uset);

        uset.max_load_factor(3);

        std::cout << "init data" << std::endl;
        int i = 0;
        for (int x = 0; x < 8; x++, i++)
        {
            uset.emplace(i);
        }
        PrintUnorderedInfo(uset);

        auto iter = uset.find(5);
        std::cout << "enum key=" << *iter << std::endl;

        std::cout << "enum equal_range" << std::endl;
        auto range = uset.equal_range(2);
        for (auto tmp = range.first; tmp != range.second; ++tmp)
        {
            std::cout << "k=" << *tmp << std::endl;
        }

        // insert invoke rehash
        for (int x = 0; x < 10; x++, i++)
        {
            uset.emplace(i);
        }

        // iter valid
        std::cout << "after first emplace, rehash£¬enum key=5 to end" << std::endl;
        for (auto tmp = iter; tmp != uset.end(); ++tmp)
        {
            std::cout << "k=" << *tmp << std::endl;
        }

        PrintUnorderedInfo(uset);

        std::cout << "emplace to 64 elems" << std::endl;
        for (int x = 0; x < 46; x++, i++)
        {
            uset.emplace(i);
        }
        PrintUnorderedInfo(uset);
        for (auto& tmp : uset)
        {
            std::cout << "bucket=" << uset.bucket(tmp) << ", \tk=" << tmp << std::endl;
        }

        //
        std::cout << "rehash to 8" << std::endl;
        uset.rehash(8);
        PrintUnorderedInfo(uset);
        for (auto& tmp : uset)
        {
            std::cout << "bucket=" << uset.bucket(tmp) << ", \tk=" << tmp << std::endl;
        }

        // 
        std::cout << "reserve to 28" << std::endl;
        uset.reserve(28);
        PrintUnorderedInfo(uset);
        for (auto& tmp : uset)
        {
            std::cout << "bucket=" << uset.bucket(tmp) << ", \tk=" << tmp << std::endl;
        }

        // force rehash
        std::cout << "force rehash" << std::endl;
        uset.rehash(0);
        PrintUnorderedInfo(uset);
        for (auto& tmp : uset)
        {
            std::cout << "bucket=" << uset.bucket(tmp) << ", \tk=" << tmp << std::endl;
        }

        // erase
        std::cout << "foreach erase" << std::endl;
        for (int i = 0; i < 64; i++)
        {
            uset.erase(i);
            PrintUnorderedInfo(uset);
        }
    }

}

void stl_container_example()
{
    unordered_map_example();
    unordered_set_example();
}