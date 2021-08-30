// ShMemAVLClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ShMemAVLTree.h"
#include "ShMemAVLClient.h"
#include <iostream>
#include <cassert>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <inttypes.h>
#if ON_WINDOWS
#else
#include <sched.h>
#include <time.h>
#endif

namespace smht
{
    ShMemAVLClient::ShMemAVLClient()
        : shmem(0)
        , avl(0)
        , valid(false)
    {
        shmem = new ShMemHolder(SH_MEM_AVL_BUFF, SH_MEM_BUFF_SIZE);
        if (shmem->IsValid())
        {
            void* BuffDataAddress = shmem->GetAddress();
            avl = new ShMemAVLTree(BuffDataAddress, SH_MEM_BUFF_SIZE, false);
            valid = true;
        }
    }

    ShMemAVLClient::~ShMemAVLClient()
    {
        if (shmem)
        {
            delete shmem;
            shmem = 0;
        }
        if (avl)
        {
            delete avl;
            avl = 0;
        }
    }

    bool ShMemAVLClient::IsValid()
    {
        return valid;
    }

    bool ShMemAVLClient::GetValue(KeyType Key, SizeType& Offset, SizeType& DataLen)
    {
        return avl->GetValue(Key, Offset, DataLen);
    }

    bool ShMemAVLClient::SetValue(KeyType Key, void* Data, SizeType DataLen)
    {
        return (avl->SetValue(Key, Data, DataLen) == SET_SUCC);
    }

    void* ShMemAVLClient::GetDataAddressByIndex(const SizeType& DataIndex)
    {
        return avl->GetDataAddressByIndex(DataIndex);
    }

}

double GetTickCountA()
{
#if ON_WINDOWS
    __int64 Freq = 0;

    __int64 Count = 0;

    if (QueryPerformanceFrequency((LARGE_INTEGER*)&Freq)

        && Freq > 0

        && QueryPerformanceCounter((LARGE_INTEGER*)&Count))

    {
        return (double)Count / (double)Freq * 1000.0;

    }
#else
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
        ).count();
#endif

    return 0.0;

}

void TestShMemAVLTree()
{

    std::cout << "Test AVL Hello World!\n";

    smht::ShMemAVLClient avl;

    double begin = GetTickCountA();

    int value = 0;
    SizeType idx = 0;
    SizeType len;
    void* res;
    int Key = 13;
    avl.SetValue(Key, &++value, sizeof(value));
    avl.GetValue(Key, idx, len);
    res = avl.GetDataAddressByIndex(idx);
    assert(*(int*)res == value);

    Key = 24;
    avl.SetValue(Key, &++value, sizeof(value));
    avl.GetValue(Key, idx, len);
    res = avl.GetDataAddressByIndex(idx);
    assert(*(int*)res == value);

    Key = 37;
    avl.SetValue(Key, &++value, sizeof(value));
    avl.GetValue(Key, idx, len);
    res = avl.GetDataAddressByIndex(idx);
    assert(*(int*)res == value);

    Key = 90;
    avl.SetValue(Key, &++value, sizeof(value));
    avl.GetValue(Key, idx, len);
    res = avl.GetDataAddressByIndex(idx);
    assert(*(int*)res == value);

    Key = 53;
    avl.SetValue(Key, &++value, sizeof(value));
    avl.GetValue(Key, idx, len);
    res = avl.GetDataAddressByIndex(idx);
    assert(*(int*)res == value);

    for(int i = 0; i < 1000000; ++i)
    {
        Key = rand();
        if (SET_SUCC == avl.SetValue(Key, &++value, sizeof(value)))
        {
            avl.GetValue(Key, idx, len);
            res = avl.GetDataAddressByIndex(idx);
            assert(*(int*)res == value);
        }
    }

    double end = GetTickCountA();

    std::cout << "Time Elapsed: " << end - begin << std::endl;
}


int main()
{
    TestShMemAVLTree();
    return 0;
}

