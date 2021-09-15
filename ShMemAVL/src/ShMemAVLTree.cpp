// ShMemAVLTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ShMemAVLTree.h"
#include <iostream>
#include <cassert>
#include <atomic>
#include <chrono>
#include <queue>
#include <iostream>
#if ON_WINDOWS
#else
#include <sched.h>
#include <time.h>
#endif

namespace smht
{

    void* AlignMemory(void* p, int bytes)
    {
        char* tp = (char*)p;
        return (void*)(((uintptr_t)(tp + bytes - 1)) & (~(uintptr_t)bytes));
    }
    /*  |-------------------------------|
    *   | RootNodeIndex                 |
    *   |-------------------------------|
    *   | CurrentBottomupTreeNodeIndex  |
    *   |-------------------------------|
    *   | CurrentTopdownDataIndex       |
    *   |-------------------------------|
    *   | SpinLockAtom                  |
    *   |-------------------------------|<--------- BaseAddress
    *   |                               |     |
    *   | ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ |     |
    *   |                               | MaxMemSize   
    *   | ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ |     |
    *   |                               |     |
    *   |-------------------------------|<-----------
    */
    ShMemAVLTree::ShMemAVLTree(void* aBaseAddress, SizeType aMaxMemSize, bool aDoInit)
        : RootNodeIndex( (SizeType*)aBaseAddress)
        , CurrentBottomupTreeNodeIndex( ((SizeType*)aBaseAddress + 1) )
        , CurrentTopdownDataIndex( ((SizeType*)aBaseAddress + 2) )
        , SpinLockAtom((char*)aBaseAddress + sizeof(SizeType) * 3 )
        , RawMemAddress(aBaseAddress)
        , BaseAddress( (char*)aBaseAddress + sizeof(SizeType) * 3 + 1 )
        , MaxMemSize( aMaxMemSize - sizeof(SizeType) * 3 - 1 )
    {
        if (aDoInit)
        {
            *RootNodeIndex = INVALID_AVL_INDEX;
            *CurrentBottomupTreeNodeIndex = 0;
            *CurrentTopdownDataIndex = MaxMemSize;
            *SpinLockAtom = 0;
        }
    }

    ShMemAVLTree::~ShMemAVLTree()
    {
    }

    void ShMemAVLTree::DebugBFSPrint()
    {
        DoDebugBFSPrint(*RootNodeIndex);
    }

    SizeType ShMemAVLTree::GetEntryNum()
    {
        return *CurrentBottomupTreeNodeIndex;
    }

    char* ShMemAVLTree::GetBottomupSize(SizeType& size)
    {
        size = *CurrentBottomupTreeNodeIndex * sizeof(TreeNode) + sizeof(SizeType) * 3 + 1;
        return (char*)RawMemAddress;
    }

    char* ShMemAVLTree::GetTopdownSize(SizeType& size)
    {
        void* pData = (char*)BaseAddress + (*CurrentTopdownDataIndex);
        void* pEnd = (char*)BaseAddress + MaxMemSize;
        size = (SizeType)((char*)pEnd - (char*)pData);
        return (char*)pData;
    }

    void ShMemAVLTree::DoDebugBFSPrint(const SizeType& NodeIndex)
    {
        std::cout << "---------------------------->>>" << std::endl;
        if(NodeIndex == INVALID_AVL_INDEX)
        {
            return;
        }
        TreeNode* pNode = GetTreeNodeByIndex(NodeIndex);
        struct NodeLevel
        {
            TreeNode* pNode;
            SizeType NodeLevel;
        };
        std::queue<NodeLevel> que;
        que.push({pNode, 0});
        SizeType CurPrintLevel = 0;
        while(que.size() > 0)
        {
            NodeLevel nl = que.front();
            que.pop();
            if (nl.pNode->LeftChildIndex != INVALID_AVL_INDEX)
            {
                que.push({ GetTreeNodeByIndex(nl.pNode->LeftChildIndex), nl.NodeLevel + 1 });
            }
            if (nl.pNode->RightChildIndex != INVALID_AVL_INDEX)
            {
                que.push({ GetTreeNodeByIndex(nl.pNode->RightChildIndex), nl.NodeLevel + 1 });
            }

            if(nl.NodeLevel > CurPrintLevel)
            {
                CurPrintLevel = nl.NodeLevel;
                std::cout << std::endl;
            }
            std::cout << nl.pNode->Key << ":" << nl.pNode->BalanceFactor << " ";
        }
        std::cout << std::endl;
        std::cout << "<<<----------------------------" << std::endl;
    }

    bool ShMemAVLTree::GetValue(KeyType Key, SizeType& Offset, SizeType& DataLen)
    {
        ByteSpinLock lock(SpinLockAtom);
        return DoGetValue(*RootNodeIndex, Key, Offset, DataLen);
    }

    bool ShMemAVLTree::DoGetValue(const SizeType& NodeIndex, KeyType Key, SizeType& Offset, SizeType& DataLen)
    {
        if(NodeIndex == INVALID_AVL_INDEX)
        {
            return false;
        }
        TreeNode* pNode = GetTreeNodeByIndex(NodeIndex);
        if(pNode->Key == Key)
        {
            DataLen = pNode->DataLength;
            Offset = pNode->DataIndex;
            return true;
        }
        else if(pNode->Key > Key)
        {
            return DoGetValue(pNode->LeftChildIndex, Key, Offset, DataLen);
        }
        else
        {
            return DoGetValue(pNode->RightChildIndex, Key, Offset, DataLen);
        }
        return false;
    }

    Status ShMemAVLTree::SetValue(KeyType Key, void* Data, SizeType DataLen)
    {
        ByteSpinLock lock(SpinLockAtom);
        bool Taller = false;
        void* pAllocTreeNode = GetTreeNodeByIndex(*CurrentBottomupTreeNodeIndex);
        void* pData = GetDataAddressByIndex(*CurrentTopdownDataIndex);
        if ((char*)pAllocTreeNode + sizeof(TreeNode) + DataLen >= pData)
        {
            return MEM_FULL;
        }
        return DoSetValue(*RootNodeIndex, Key, Data, DataLen, Taller);
    }

    Status ShMemAVLTree::DoSetValue(SizeType& NodeIndex, KeyType Key, void* Data, SizeType DataLen, bool& Taller)
    {
        if(NodeIndex == INVALID_AVL_INDEX)
        {
            SizeType NewNodeIndex = AllocateTreeNode();
            TreeNode* pNode = GetTreeNodeByIndex(NewNodeIndex);
            pNode->Key = Key;
            SizeType DataIndex = AllocateData(DataLen);
            void* pData = GetDataAddressByIndex(DataIndex);
            memcpy(pData, Data, DataLen);
            pNode->DataIndex = DataIndex;
            pNode->DataLength = DataLen;
            NodeIndex = NewNodeIndex;
            Taller = true;
            return SET_SUCC;
        }
        else
        {
            TreeNode* pNode = GetTreeNodeByIndex(NodeIndex);
            if (pNode->Key == Key)
            {
                return KEY_EXISTS;
            }
            else if (pNode->Key > Key)
            {
                SizeType& LeftChildIndex = pNode->LeftChildIndex;
                if (DoSetValue(LeftChildIndex, Key, Data, DataLen, Taller) == KEY_EXISTS)
                {
                    return KEY_EXISTS;
                }
                else
                {
                    if (Taller)
                    {
                        if (pNode->BalanceFactor == LEFT_HIGH)
                        {
                            LeftBalance(NodeIndex);
                            Taller = false;
                        }
                        else if (pNode->BalanceFactor == EQUAL_HIGH)
                        {
                            pNode->BalanceFactor = LEFT_HIGH;
                            Taller = true;
                        }
                        else
                        {
                            pNode->BalanceFactor = EQUAL_HIGH;
                            Taller = false;
                        }
                    }
                }
            }
            else
            {
                SizeType& RightChildIndex = pNode->RightChildIndex;
                if (DoSetValue(RightChildIndex, Key, Data, DataLen, Taller) == KEY_EXISTS)
                {
                    return KEY_EXISTS;
                }
                else
                {
                    if (Taller)
                    {
                        if (pNode->BalanceFactor == LEFT_HIGH)
                        {
                            pNode->BalanceFactor = EQUAL_HIGH;
                            Taller = false;
                        }
                        else if (pNode->BalanceFactor == EQUAL_HIGH)
                        {
                            pNode->BalanceFactor = RIGHT_HIGH;
                            Taller = true;
                        }
                        else
                        {
                            RightBalance(NodeIndex);
                            Taller = false;
                        }
                    }
                }
            }
        }
        return SET_SUCC;
    }

    TreeNode* ShMemAVLTree::GetTreeNodeByIndex(const SizeType& Index)
    {
        return (TreeNode*)BaseAddress + Index;
    }

    void* ShMemAVLTree::GetDataAddressByIndex(const SizeType& DataIndex)
    {
        return (char*)BaseAddress + DataIndex;
    }

    SizeType ShMemAVLTree::AllocateTreeNode()
    {
        TreeNode* p = new((TreeNode*)BaseAddress + *CurrentBottomupTreeNodeIndex) TreeNode;
        return (*CurrentBottomupTreeNodeIndex)++;
    }

    SizeType ShMemAVLTree::AllocateData(const SizeType& DataLength)
    {
        return *CurrentTopdownDataIndex -= DataLength;
    }

    void ShMemAVLTree::LeftBalance(SizeType& NodeIndex)
    {
        TreeNode* pNode = GetTreeNodeByIndex(NodeIndex);
        SizeType& LeftChildIndex = pNode->LeftChildIndex;
        TreeNode* pLeftChild = GetTreeNodeByIndex(LeftChildIndex);
        if(pLeftChild->BalanceFactor == LEFT_HIGH)
        {
            pNode->BalanceFactor = pLeftChild->BalanceFactor = EQUAL_HIGH;
            R_Rotate(NodeIndex);
        }
        else if(pLeftChild->BalanceFactor == RIGHT_HIGH)
        {
            SizeType& RightChildIndex = pLeftChild->RightChildIndex;
            TreeNode* pRightChild = GetTreeNodeByIndex(RightChildIndex);
            if(pRightChild->BalanceFactor == LEFT_HIGH)
            {
                pNode->BalanceFactor = RIGHT_HIGH;
                pLeftChild->BalanceFactor = EQUAL_HIGH;
            }
            else if(pRightChild->BalanceFactor == EQUAL_HIGH)
            {
                pNode->BalanceFactor = EQUAL_HIGH;
                pLeftChild->BalanceFactor = EQUAL_HIGH;
            }
            else
            {
                pNode->BalanceFactor = EQUAL_HIGH;
                pLeftChild->BalanceFactor = LEFT_HIGH;
            }
            pRightChild->BalanceFactor = EQUAL_HIGH;
            L_Rotate(LeftChildIndex);
            R_Rotate(NodeIndex);
        }
    }

    void ShMemAVLTree::RightBalance(SizeType& NodeIndex)
    {
        TreeNode* pNode = GetTreeNodeByIndex(NodeIndex);
        SizeType& RightChildIndex = pNode->RightChildIndex;
        TreeNode* pRightChild = GetTreeNodeByIndex(RightChildIndex);
        if(pRightChild->BalanceFactor == RIGHT_HIGH)
        {
            pNode->BalanceFactor = EQUAL_HIGH;
            pRightChild->BalanceFactor = EQUAL_HIGH;
            L_Rotate(NodeIndex);
        }
        else if(pRightChild->BalanceFactor == LEFT_HIGH)
        {
            SizeType& LeftChildIndex = pRightChild->LeftChildIndex;
            TreeNode* pLeftChild = GetTreeNodeByIndex(LeftChildIndex);
            if(pLeftChild->BalanceFactor == LEFT_HIGH)
            {
                pNode->BalanceFactor = EQUAL_HIGH;
                pRightChild->BalanceFactor = RIGHT_HIGH;
            }
            else if(pLeftChild->BalanceFactor == EQUAL_HIGH)
            {
                pNode->BalanceFactor = EQUAL_HIGH;
                pRightChild->BalanceFactor = EQUAL_HIGH;
            }
            else
            {
                pNode->BalanceFactor = EQUAL_HIGH;
                pRightChild->BalanceFactor = LEFT_HIGH;
            }
            R_Rotate(RightChildIndex);
            L_Rotate(NodeIndex);
        }
    }

    void ShMemAVLTree::R_Rotate(SizeType& NodeIndex)
    {
        TreeNode* p = GetTreeNodeByIndex(NodeIndex);
        SizeType LeftChildIndex = p->LeftChildIndex;
        TreeNode* lc = GetTreeNodeByIndex(LeftChildIndex);
        p->LeftChildIndex = lc->RightChildIndex;
        lc->RightChildIndex = NodeIndex;
        NodeIndex = LeftChildIndex;
    }

    void ShMemAVLTree::L_Rotate(SizeType& NodeIndex)
    {
        TreeNode* p = GetTreeNodeByIndex(NodeIndex);
        SizeType RightChildIndex = p->RightChildIndex;
        TreeNode* rc = GetTreeNodeByIndex(RightChildIndex);
        p->RightChildIndex = rc->LeftChildIndex;
        rc->LeftChildIndex = NodeIndex;
        NodeIndex = RightChildIndex;
    }
}


