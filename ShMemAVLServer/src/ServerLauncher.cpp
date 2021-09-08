#include "ShMemUtils.h"
#include <iostream>
#include <memory>
#include <inttypes.h>

#if ON_WINDOWS
#define SERVER_PROC_NAME "ShMemAVLServer.exe"
#else
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#define SERVER_PROC_NAME "ShMemAVLServer"
#endif

int main()
{
    if(smht::CheckProcAliveByName(SERVER_PROC_NAME))
    {
        std::cout << "ShMemServer exists!" << std::endl;
        return 0;
    }

#if ON_WINDOWS
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
        SERVER_PROC_NAME,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return 0;
    }
    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    int status;
    int pid = 0;
    pid = fork();
    if(pid == -1)
    {
        perror("fork");
        exit(-1);
    }
    if(pid > 0)
    {
        printf("Parent exit!\n");
        exit(0);
    }
    if(pid == 0)
    {
        execl("./" SERVER_PROC_NAME, "ShMemAVLServer", (char*)0);
        printf("Child exited with status %i\n", status);
        return 0;
    }
#endif
}
