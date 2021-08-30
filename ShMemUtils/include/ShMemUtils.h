#pragma once

// platform-specific headers.
#if defined(_MSC_VER)
# define ON_WINDOWS		1
# include <windows.h>
#undef UNICODE
#include <tlhelp32.h>
#define UNICODE
#else
# define ON_WINDOWS		0
# include <unistd.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <string.h>
# include <linux/kernel.h>
# include <dirent.h>
#endif

#include <cstdint>
#include <atomic>
#include <string>

#define DEBUG                   1
#define DebugPrint(fmt, ...)    do { if (DEBUG) fprintf(stderr, "File:%s, Line:%d\n" fmt, __FILE__, __LINE__, ##__VA_ARGS__ ); } while (0)

#define  KeyType   uint64_t
#define  SizeType  uint32_t

#define MAX_PATH_LEN           254
#define SH_MEM_BUFF_SIZE       1024 * 1024 * 1024
#define SH_MEM_AVL_BUFF        "SH_MEM_AVL_BUFF"
#define FILE_PERMISSION        S_IRWXU

namespace smht
{
    char                TestAndSwapByteVal(char volatile* ptr, char expected, char newval);

    bool                CAllocAndClone(char*& Dest, const char* Source);
    void                MemoryFence();
    bool                CheckProcAliveByName(const char* ProcName);

    class ByteSpinLock
    {
    public:
        ByteSpinLock(char volatile* ptr)
            : _ptr(ptr)
        {
            while (TestAndSwapByteVal(_ptr, 0, 1) != 0);
        }
        ~ByteSpinLock()
        {
            while (TestAndSwapByteVal(_ptr, 1, 0) != 1);
        }
    private:
        char volatile* _ptr;
    };

    class ShMemHolder
    {
    public:
        explicit ShMemHolder(const char* name, uint32_t size);
        ~ShMemHolder();

    private:
        void				CreateOrOpen();
        void				Close();

    public:
        bool 				IsValid();
        const char*			GetName();
        uint32_t			GetSize();
        void*         		GetAddress();

    private:
        ShMemHolder(const ShMemHolder&);
        ShMemHolder(const ShMemHolder&&);

    private:
        char*				Name;
        char* 				FullShMemName;
        uint32_t			Size;
        #if ON_WINDOWS
        void*       		Handle;
        #else
        int                 Handle;
        #endif
        void*       		MappedAddress;
        bool				Initialized;
    };
}
