#include "stdafx.h"
#include <algorithm>
#include <vector>
#include <excpt.h>


struct PublishedFileInfo
{
    enum PublishedFileHashType
    {
        md5,
        sha1,
    };
    std::string relate_path;// 相对路径
    int hash_type = PublishedFileHashType::md5;  // 文件hash类型
    std::string hash;   // 文件hash值
    __int64 length = 0; // 文件大小

    bool operator ==(const PublishedFileInfo& _Right) const
    {
        return relate_path == _Right.relate_path &&
            hash_type == _Right.hash_type &&
            hash == _Right.hash &&
            length == _Right.length;
    }
};

void catch_error()
{
    PublishedFileInfo* ptr = 0;
    ptr->hash_type = 9;
}

void try_except_test()
{
    __try
    {
        catch_error();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("asdsa");
    }
}

void vector_operation_study()
{
    PublishedFileInfo* ptr = 0;
    try
    {
        ptr->hash_type = 9;
    }
    catch (...)
    {
        printf("asdsa");
    }
    

    std::vector<PublishedFileInfo> local_files{
        {"a.exe", 0, "hash", 1234},
        {"c.exe", 0, "hashC", 1234},
        {"g.exe", 0, "hashG", 1234},
        {"j.exe", 0, "hashJ", 1234},
        {"l.exe", 0, "hashL", 1234}
    };
    std::vector<PublishedFileInfo> remote_files{
        {"a.exe", 0, "hash", 1234},
        {"b.exe", 0, "hashB", 5678},
        {"e.exe", 0, "hashE", 5678},
        {"g.exe", 0, "hashG", 5678},
        {"h.exe", 0, "hashH", 5678},
        {"j.exe", 0, "hashJ", 5678},
        {"k.exe", 0, "hashK", 5678}
    };

    // 求差集前必须先将集合排序，按相对路径来排
    std::sort(local_files.begin(), local_files.end(), 
        [](const PublishedFileInfo& v1, const PublishedFileInfo& v2)->bool
        {
            return v1.relate_path < v2.relate_path;
        }
    );
    std::sort(remote_files.begin(), remote_files.end(), 
        [](const PublishedFileInfo& v1, const PublishedFileInfo& v2)->bool
        {
            return v1.relate_path < v2.relate_path;
        }
    );

    // 直接求远端发布文件列表对本地文件列表的差集，求普通差集即可，不需要对称差集；
    // 即在远端有在本地没有的则把第一个集合的项插入结果集合，全部数据以远端的为准，所以remote_files必须作为第一个集合参数；
    // 得出的以远端数据为基础的差集是必须更新项，不需要做额外的比较判断了
    std::vector<PublishedFileInfo> diff_files;
    std::set_difference(remote_files.begin(), remote_files.end(), local_files.begin(), local_files.end(),
        std::back_inserter(diff_files),
        [](const PublishedFileInfo& r_file, const PublishedFileInfo& l_file)->bool
        {
            return r_file.relate_path < l_file.relate_path;
        }
    );

    // 求交集，有相同的则把第一个集合的项插入结果集合，全部数据以远端的为准，所以remote_files必须作为第一个集合参数
    std::vector<PublishedFileInfo> itsct_files;
    std::set_intersection(remote_files.begin(), remote_files.end(), local_files.begin(), local_files.end(),
        std::back_inserter(itsct_files),
        [](const PublishedFileInfo& r_file, const PublishedFileInfo& l_file)->bool
        {
            return r_file.relate_path < l_file.relate_path;
        }
    );

    // 将交集进行遍历对本地文件列表进行比较，HASH不一致的必须更新
    std::vector<PublishedFileInfo> copy_files;
    for (auto& r_file : itsct_files)
    {
        auto find_ret = std::find_if(local_files.begin(), local_files.end(),
            [&](const PublishedFileInfo& l_file)->bool
            {
                return r_file == l_file;
            }
        );

        // 路径相同的交集项文件不一致那就需要更新，
        // 一致的直接拷贝一份即可
        if (find_ret == local_files.end())
        {
            diff_files.push_back(r_file);
        }
        else
        {
            copy_files.push_back(r_file);
        }
    }

    // 至此，本地文件列表和远端文件列表对比结束，对比得出两个结果集:
    // diff_files：结果集里记录的是需要下载更新的项（远端新增的或是远端有修改的）；
    // copy_files：结果集里记录的是不需要下载更新的项，直接拷贝一份到新版本目标目录即可；
}