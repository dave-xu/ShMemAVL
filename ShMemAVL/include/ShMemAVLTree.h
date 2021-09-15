#pragma once

#include "ShMemUtils.h"
#include <memory>

namespace smht
{

    struct KeyEntry
    {
        SizeType DataOffset;
        SizeType DataLength;
    };

    void*               AlignMemory(void* p, int bytes);

    #define LEFT_HIGH           +1
    #define EQUAL_HIGH          0
    #define RIGHT_HIGH          -1
    #define INVALID_AVL_INDEX   -1

    #define Status              uint32_t
    #define KEY_EXISTS          0
    #define SET_SUCC            1
    #define SET_FAIL            2
    #define MEM_FULL            -1

    struct TreeNode
    {
        KeyType     Key;
        SizeType    BalanceFactor;
        SizeType    LeftChildIndex;
        SizeType    RightChildIndex;

        SizeType    DataIndex;
        SizeType    DataLength;

        TreeNode()
        : Key(0)
        , BalanceFactor(EQUAL_HIGH)
        , LeftChildIndex(INVALID_AVL_INDEX)
        , RightChildIndex(INVALID_AVL_INDEX)
        , DataIndex(INVALID_AVL_INDEX)
        , DataLength(0)
        {
        }
    };

    class ShMemAVLTree
    {


    public:
        explicit ShMemAVLTree(void* aBaseAddress, SizeType aMaxMemSize, bool aDoInit);
        virtual ~ShMemAVLTree();

    public:
        bool            GetValue(KeyType Key, SizeType& Offset, SizeType& DataLen);
        Status          SetValue(KeyType Key, void* Data, SizeType DataLen);
        void*           GetDataAddressByIndex(const SizeType& DataIndex);

        void            DebugBFSPrint();
        SizeType        GetEntryNum();

    private:
        ShMemAVLTree(const ShMemAVLTree&);
        ShMemAVLTree(const ShMemAVLTree&&);

    private:
        bool DoGetValue(const SizeType& NodeIndex, KeyType Key, SizeType& Offset, SizeType& DataLen);
        Status DoSetValue(SizeType& NodeIndex, KeyType Key, void* Data, SizeType DataLen, bool& Taller);
        void DoDebugBFSPrint(const SizeType& NodeIndex);
        TreeNode* GetTreeNodeByIndex(const SizeType& Index);

        SizeType AllocateTreeNode();
        SizeType AllocateData(const SizeType& DataLength);

        void LeftBalance(SizeType& NodeIndex);
        void RightBalance(SizeType& NodeIndex);
        void R_Rotate(SizeType& NodeIndex);
        void L_Rotate(SizeType& NodeIndex);

    public:
        char* GetBottomupSize(SizeType& size);
        char* GetTopdownSize(SizeType& size);

    public:
        char volatile*                   SpinLockAtom;
        SizeType*                        RootNodeIndex;
        // Keys grow from low address to high address;
        // while Data grows just the other way.
        SizeType*                        CurrentBottomupTreeNodeIndex;
        SizeType*                        CurrentTopdownDataIndex;

    private:
        void*                           RawMemAddress;
        void*                           BaseAddress;
        SizeType                        MaxMemSize;
    };
}
