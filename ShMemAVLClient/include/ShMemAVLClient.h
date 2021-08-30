#pragma once

#include "ShMemAVLTree.h"
#include <memory>

namespace smht
{
    class ShMemAVLClient
    {
    public:
        explicit ShMemAVLClient();
        virtual ~ShMemAVLClient();

    public:
        bool            IsValid();
        bool            GetValue(KeyType Key, SizeType& Offset, SizeType& DataLen);
        bool            SetValue(KeyType Key, void* Data, SizeType DataLen);
        void*           GetDataAddressByIndex(const SizeType& DataIndex);

    private:
        ShMemAVLClient(const ShMemAVLClient&);
        ShMemAVLClient(const ShMemAVLClient&&);

    private:
        bool valid;
        ShMemHolder* shmem;
        ShMemAVLTree* avl;
    };
}
