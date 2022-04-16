#include "stdafx.h"
#include <vector>
#include <utility>

using namespace std;

const int asize = 10;

void PrintArray(const int* a, int len)
{
    for (int i = 0; i < len; i++)
    {
        cout << *(a + i) << ",";
    }
    cout << endl;
}

// 快排
void Quicksort(int a[], int left, int right) {// l 代表排序开始位置；e 代表排序终止位置。
    if (left >= right) return;// 保证开始位置在终止位置之前。
    int i = left, j = right;// 利用始末位置而不改变原值：所以增加两个新变量。
    int key = left;// key类似指针，开始时指向初始位置，即假设第一个数是有序的。
    cout << "关键数据 " << a[key] << " ：" << endl;
    while (j > i)
    {
        // while (j > i) 的每一次循环是为了确定：
        // 从左往右的方向上，第一个比参照数小的数的位置；
        // 从右往左的方向上，第一个比参照数大的数的位置；

        while (j > i && a[j] >= a[key])
        {
            j--;// 必须从右边的在前面
        }

        while (j > i && a[i] <= a[key])
        {
            i++;// 从前面得知，key == i，如果这两行在前面，那一定会先执行这个操作，这样导致从右往左的对比会少了一个数
        }

        if (i != j)
        {
            cout << "本次交换 " << a[i] << "[" << i << "]" << "-" << a[j] << "[" << j << "]" << " ：";
            swap(a[i], a[j]);// swap是一个简单的交换函数（偷下懒qaq）

            PrintArray(a, asize);
        }
    }// i == j 时跳出while循环

     // 如果不能理解建议在自己手画模拟一遍
    if (key != i)
    {
        swap(a[key], a[i]);// 以此时的a[i] 为分界线，前面的数一定比a[i]小，后面的一定比a[i]大

        cout << "本次分区 " << a[i] << "[" << i << "]" << " ：";
        PrintArray(a, asize);
    }

    // 再运用递归将a[i]前面的和后面的分别排好序
    Quicksort(a, left, i - 1);
    Quicksort(a, i + 1, right);
}

// 二分查找
int BinarySearch1(int a[], int value, int n)
{
    int low, high, mid;
    low = 0;
    high = n - 1;
    while (low <= high)
    {
        mid = (low + high) / 2;
        if (a[mid] == value)
            return mid;
        if (a[mid] > value)
            high = mid - 1;
        if (a[mid] < value)
            low = mid + 1;
    }
    return -1;
}

//二分查找，递归版本
int BinarySearch2(int a[], int value, int low, int high)
{
    int mid = low + (high - low) / 2;
    if (a[mid] == value)
        return mid;
    if (a[mid] > value)
        return BinarySearch2(a, value, low, mid - 1);
    if (a[mid] < value)
        return BinarySearch2(a, value, mid + 1, high);
    return -1;
}

// 获取整数各个位数上的值，从个位开始返回
std::vector<int> GetIntegerDigit(int i)
{
    if (0 >= i)
    {
        return {};
    }

    std::vector<int> vct;
    int num = 0;// 多少数位
    int n = i;
    int c = 0;
    while (n > 0)
    {
        c = n % 10;

        vct.push_back(c);

        n = n / 10;
    }
    return vct;
}

void TestSort()
{
    int value;
    std::srand((unsigned int)time(0));
    int a[asize] = { 0 };
    for (int i = 0; i < asize; ++i)
    {
        a[i] = std::rand() % 100 + 1;

        if (i == 5)
        {
            value = a[i];
        }
    }

    cout << "初始数组：" << endl;
    PrintArray(a, asize);

    cout << "快排：" << endl;
    Quicksort(a, 0, asize - 1);
    cout << "排序完毕：" << endl;
    PrintArray(a, asize);


    int idx = BinarySearch1(a, value, asize);
    cout << "二分查找：" << value << "[" << idx << "]" << endl;


    int i = 23456;
    auto vct = GetIntegerDigit(i);
    cout << "计算数位：" << i << endl;
    for (auto& n : vct)
    {
        cout << n << ",";
    }
    cout << endl;
}
