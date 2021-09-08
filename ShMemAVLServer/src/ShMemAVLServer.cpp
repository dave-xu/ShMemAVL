// ShMemAVLTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ShMemAVLTree.h"
#include <iostream>
#include <cassert>
#include <atomic>
#include <chrono>
#include <inttypes.h>
#if ON_WINDOWS
#define SERVER_PROC_NAME "ShMemServer.exe"
#else
#include <time.h>
#define SERVER_PROC_NAME "ShMemServer"
#endif

int AllocShMemAVL()
{
    std::cout << "Hello World!\n";

    std::shared_ptr<smht::ShMemHolder> SlotMemPtr = std::make_shared<smht::ShMemHolder>(SH_MEM_AVL_BUFF, SH_MEM_BUFF_SIZE);
    DebugPrint("SlotMemHolder:|%s|\n", SlotMemPtr->GetName());
    if (SlotMemPtr->IsValid())
    {
        DebugPrint("SlotData create succ.\n");
        void* BuffDataAddress = SlotMemPtr->GetAddress();
        DebugPrint("CreateSlotDataAddress:|%s|%" PRId64 "|\n", SlotMemPtr->GetName(), (int64_t)BuffDataAddress);
        smht::ShMemAVLTree avl(BuffDataAddress, SH_MEM_BUFF_SIZE, true);
        while (true)
        {
            
#if ON_WINDOWS
            std::cout << "Current Entry Number: " << avl.GetEntryNum() << std::endl;
            Sleep(3000);
#else
            static struct timespec ts;
            {
                ts.tv_sec = 3;
                ts.tv_nsec = 0;
                nanosleep(&ts, &ts);
            }
#endif
        }
        return 0;
    }
    else
    {
        DebugPrint("SlotData create failed.\n");
        return -1;
    }
    
}

int main()
{
    return AllocShMemAVL();
    return 0;
}