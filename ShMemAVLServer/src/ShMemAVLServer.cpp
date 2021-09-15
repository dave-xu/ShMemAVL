// ShMemAVLTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ShMemAVLTree.h"
#include "ShMemUtils.h"
#include <cstdio>
#include <iostream>
#include <cassert>
#include <atomic>
#include <chrono>
#include <inttypes.h>
#if ON_WINDOWS
#define SERVER_DUMP_FILE "ShMemAVLTreeDump"
#define SERVER_DUMP_FILE_OLD "ShMemAVLTreeDumpOLD"
#define SERVER_DUMP_FILE_NEW "ShMemAVLTreeDumpNEW"
#else
#include <time.h>
#define SERVER_DUMP_FILE "ShMemAVLTreeDump"
#define SERVER_DUMP_FILE_OLD "ShMemAVLTreeDumpOLD"
#define SERVER_DUMP_FILE_NEW "ShMemAVLTreeDumpNEW"
#endif

#define MAGIC_NUM 0xAABBCCDD


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
        bool hasDumpData = false;

        FILE* fp = fopen(SERVER_DUMP_FILE, "rb");
        if (fp)
        {
            int32_t MagicNum = 0;
            size_t ret = fread(&MagicNum, 1, sizeof(MagicNum), fp);
            if (ret == sizeof(MagicNum) && MagicNum == MAGIC_NUM)
            {
                SizeType NumFrontBytes = 0;
                SizeType NumBackBytes = 0;
                ret = fread(&NumFrontBytes, 1, sizeof(SizeType), fp);
                if (ret == sizeof(SizeType))
                {
                    char* front = new char[NumFrontBytes];
                    char* p = front;
                    SizeType toread = NumFrontBytes;
                    while (ret = fread(p, 1, toread, fp))
                    {
                        toread -= ret;
                        p += ret;
                    }
                    if (toread == 0)
                    {
                        ret = fread(&NumBackBytes, 1, sizeof(SizeType), fp);
                        if (ret == sizeof(SizeType))
                        {
                            char* back = new char[NumBackBytes];
                            ret = fread(back, 1, NumFrontBytes, fp);
                            if (ret == NumFrontBytes)
                            {
                                memcpy(BuffDataAddress, front, NumFrontBytes);
                                char* BackOffset = (char*)BuffDataAddress + SH_MEM_BUFF_SIZE - NumBackBytes;
                                memcpy(BackOffset, back, NumBackBytes);
                                hasDumpData = true;
                                delete[] front;
                                delete[] back;
                            }
                            else
                            {
                                delete[] front;
                                delete[] back;
                            }
                        }
                        else
                        {
                            delete[] front;
                        }
                    }
                    else
                    {
                        delete[] front;
                    }
                }
            }
            fclose(fp);
        }
        

        smht::ShMemAVLTree avl(BuffDataAddress, SH_MEM_BUFF_SIZE, !hasDumpData);
        while (true)
        {

            FILE* fp = fopen(SERVER_DUMP_FILE_NEW, "wb");
            if (fp)
            {
                int num = avl.GetEntryNum();
                SizeType frontsize = 0;
                char* front = avl.GetBottomupSize(frontsize);
                SizeType backsize = 0;
                char* back = avl.GetTopdownSize(backsize);
                int32_t Magic = 0;
                size_t ret = fwrite(&Magic, 1, sizeof(Magic), fp);
                if (ret == sizeof(Magic))
                {
                    ret = fwrite(&frontsize, 1, sizeof(frontsize), fp);
                    if (ret == sizeof(frontsize))
                    {
                        ret = fwrite(front, 1, frontsize, fp);
                        if (ret == frontsize)
                        {
                            ret = fwrite(&backsize, 1, sizeof(backsize), fp);
                            if (ret == sizeof(backsize))
                            {
                                ret = fwrite(back, 1, backsize, fp);
                                if (ret == backsize)
                                {
                                    fseek(fp, 0, SEEK_SET);
                                    Magic = MAGIC_NUM;
                                    fwrite(&Magic, 1, sizeof(Magic), fp);
                                }
                            }
                        }
                    }
                }
                fflush(fp);
                fclose(fp);
            }
            

            remove(SERVER_DUMP_FILE_OLD);
            rename(SERVER_DUMP_FILE, SERVER_DUMP_FILE_OLD);
            rename(SERVER_DUMP_FILE_NEW, SERVER_DUMP_FILE);
        
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