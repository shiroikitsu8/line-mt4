#include "yac.h"
#include <stdio.h>
#include <unistd.h>
#include <Windows.h>
#include "shared/line.h"

HANDLE YAC_PIPE;

int YAC_FD = 19421023;

int(__cdecl *old_open)(const char *file, int oflag, ...);
int jmp_open(const char *file, int oflag, ...)
{
    printf("%s %d\n", file, oflag);
    if (strcmp(file, "/dev/ttyS1") == 0)
        return YAC_FD;

    int ret;
    if (oflag & 00000100) // O_CREAT
    {
        va_list arg;
        va_start(arg, oflag);
        int mode = va_arg(arg, int);
        va_end(arg);

        ret = old_open(file, oflag, mode);
    }
    else
    {
        ret = old_open(file, oflag);
    }

    return ret;
}

int(__cdecl *old_ioctl)(int fd, int op, void *arg);
int jmp_ioctl(int fd, int op, void *arg)
{
    if (fd == YAC_FD) {
        printf("ioctl\n");
        return 0;
    }
    return old_ioctl(fd, op, arg);
}

int(__cdecl *old_tcgetattr)(int fd, void *attr);
int jmp_tcgetattr(int fd, void *attr)
{
    if (fd == YAC_FD) {
        
    printf("tcgetattr\n");
        return 0;
    }
    return old_tcgetattr(fd, attr);
}

int(__cdecl *old_tcsetattr)(int fd, void *attr);
int jmp_tcsetattr(int fd, void *attr)
{
    if (fd == YAC_FD) {
        
    printf("tcsetattr\n");
        return 0;
    }
    return old_tcsetattr(fd, attr);
}

int(__cdecl *old_read)(int fd, void *buf, size_t count);
int jmp_read(int fd, void *buf, size_t count)
{
    if (fd == YAC_FD)
    {
        DWORD read;
        if (!ReadFile(YAC_PIPE, buf, count, &read, NULL))
        {
            printf("Error writing named pipe\n");
            return -1;
        }
        return read;
    }
    return old_read(fd, buf, count);
}

int(__cdecl *old_write)(int fd, void *buf, size_t count);
int jmp_write(int fd, void *buf, size_t count)
{
    if (fd == YAC_FD)
    {
        DWORD written;
        if (!WriteFile(YAC_PIPE, buf, count, &written, NULL))
        {
            printf("Error writing named pipe\n");
            return -1;
        }
        return written;
    }
    return old_write(fd, buf, count);
}

int(__cdecl *old_close)(int fd);
int jmp_close(int fd)
{
    if (fd == YAC_FD)
    {
        printf("close yac fd\n");
        return 0;
    }
    return old_close(fd);
}

void yac_init()
{
    while (1)
    {
        YAC_PIPE = CreateFileA(
            "\\\\.\\pipe\\YACardEmu", // pipe name
            GENERIC_READ |            // read and write access
                GENERIC_WRITE,
            0,             // no sharing
            NULL,          // default security attributes
            OPEN_EXISTING, // opens existing pipe
            0,             // default attributes
            NULL);         // no template file

        if (YAC_PIPE != INVALID_HANDLE_VALUE)
            break;

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            printf("Cannot open yacardemu pipe\n");
            return;
        }

        if (!WaitNamedPipeA("\\\\.\\pipe\\YACardEmu", 30000))
        {
            printf("Could not open pipe: 30 second wait timed out.");
            return;
        }
    }

    Line::Hook(Line::ResolveStub("open"), (void *)jmp_open, (void **)&old_open);

    Line::Hook(Line::ResolveStub("ioctl"), (void *)jmp_ioctl, (void **)&old_ioctl);

    Line::Hook(Line::ResolveStub("tcgetattr"), (void *)jmp_tcgetattr, (void **)&old_tcgetattr);
    Line::Hook(Line::ResolveStub("tcsetattr"), (void *)jmp_tcsetattr, (void **)&old_tcsetattr);

    Line::Hook(Line::ResolveStub("read"), (void *)jmp_read, (void **)&old_read);
    Line::Hook(Line::ResolveStub("write"), (void *)jmp_write, (void **)&old_write);
    
    Line::Hook(Line::ResolveStub("close"), (void *)jmp_close, (void **)&old_close);
}