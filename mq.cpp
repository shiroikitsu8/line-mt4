#include "mq.h"
#include <unordered_map>
#include <errno.h>
#include <string.h>

int replyFd = 1;
int commandFd = 2;

#pragma push(pack, 1)
struct linux_mq_attr
{
    long mq_flags;   /* Flags: 0 or O_NONBLOCK */
    long mq_maxmsg;  /* Max. # of messages on queue */
    long mq_msgsize; /* Max. message size (bytes) */
    long mq_curmsgs; /* # of messages currently in queue */
};
#pragma pop(pack)

int jmp_mq_open(const char *name, int oflag, int mode, void *arg)
{
    if (strcmp(name, "/Sys.Monitor.Command") == 0)
    {
        errno = 0;
        return commandFd;
    }
    if (strcmp(name, "/Sys.Monitor.Reply") == 0)
    {
        errno = 0;
        return replyFd;
    }
    errno = EACCES;
    return -1;
}

int jmp_mq_getattr(int fd, linux_mq_attr *attr)
{
    attr->mq_msgsize = 0xffff;
    
    errno = 0;
    return 0;
}

int jmp_mq_notify(int fd, void *sevp)
{
    errno = 0;
    return 0;
}

int jmp_mq_send(int fd, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
{
    errno = 0;
    return 0;
}

int jmp_mq_receive(int fd, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
    errno = EAGAIN;
    return -1;
}

int jmp_mq_close(int fd)
{
    errno = 0;
    return 0;
}

std::unordered_map<const char *, void *> MQ_FUNCS_STUB = {
    {"mq_open", (void *)jmp_mq_open},
    {"mq_getattr", (void *)jmp_mq_getattr},
    {"mq_notify", (void *)jmp_mq_notify},
    {"mq_send", (void *)jmp_mq_send},
    {"mq_receive", (void *)jmp_mq_receive},
    {"mq_close", (void *)jmp_mq_close}};

bool mq_resolve(const char *symbol, void **result)
{
    for (auto &it : MQ_FUNCS_STUB)
    {
        if (strcmp(it.first, symbol) == 0)
        {
            *result = it.second;
            return true;
        }
    }
    return false;
}