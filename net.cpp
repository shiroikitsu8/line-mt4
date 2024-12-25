#include "net.h"
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <endianness.h>
#include "shared/line.h"
#include "shared/config.h"

#pragma pack(push, 1)
enum class NetworkCheck : uint32_t
{
    NONE = 0,
    INTERFACE = 1,
    CABLE = 2,
    GATEWAY = 3,
    SHOP_ADDRESS = 4,
    HOPS = 5,
    AUTH = 6,
    SYNC_DATE = 7,
    DONE = 8,
    RENEW = 9,
    TEST = 10,
    WIFI = 11,
    SET_DATE = 12,
    ONLINE = 13,
    CABLE_CHECK = 14, // wth?? 2 cable check
    NTP = 15,
    PKG_FILE = 16
};

enum class NetworkState
{
    NONE = 0,
    BUSY = 1,
    BAD = 2,
    GOOD = 3
};

struct Sys_Network_Impl
{
    char pad[436];
    NetworkState resolveState; // 436
    char pad2[104]; // 440
    NetworkState renewState;    // 544
    NetworkState syncDateState; // 548
    NetworkState cableState;   // 552
    NetworkState gatewayState; // 556
    NetworkState contentRouterState; // 560
    NetworkState shopRouterState; // 564
    NetworkState hopsState; // 568
    char pad3[4]; // 572
    uint32_t hops;          // 576
    NetworkState wifiRouterState; // 580
    NetworkState onlineState;     // 584
    NetworkState setDateState;    // 588
    NetworkState ntpState;        // 592
    NetworkState pkgFileState;    // 596
    char pad4[28]; // 600
    NetworkCheck check;           // 628
};

struct Sys_Network
{
    Sys_Network_Impl *impl;
};

#pragma pack(pop)

bool jmp_Sys_Net_interface_isAddressChange(void *a1)
{
    return false;
}

Sys_Network *(__cdecl *Sys_Network_GetInstance)();
void(__cdecl *Sys_Network_Impl_updateState)(Sys_Network_Impl *network, void *state_array);
void jmp_Sys_Network_UpdateState(void *state_array)
{
    *(void **)&Sys_Network_GetInstance = (void *)0x80a5c60;
    *(void **)&Sys_Network_Impl_updateState = (void *)0x80a9b70;

    Sys_Network_Impl *network_impl = Sys_Network_GetInstance()->impl;

    switch (network_impl->check)
    {
    case NetworkCheck::CABLE:
    {
        network_impl->cableState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::GATEWAY:
    {
        network_impl->gatewayState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::SHOP_ADDRESS:
    {
        network_impl->resolveState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::HOPS:
    {
        network_impl->hops = 1;
        network_impl->hopsState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::SYNC_DATE:
    {
        network_impl->syncDateState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::RENEW:
    {
        network_impl->renewState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::TEST:
    {
        network_impl->cableState = NetworkState::GOOD;
        network_impl->shopRouterState = NetworkState::GOOD;
        network_impl->contentRouterState = NetworkState::GOOD;
        network_impl->hopsState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::WIFI:
    {
        network_impl->wifiRouterState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::SET_DATE:
    {
        network_impl->setDateState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::ONLINE:
    {
        network_impl->onlineState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::CABLE_CHECK:
    {
        network_impl->cableState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::NTP:
    {
        network_impl->ntpState = NetworkState::GOOD;
        break;
    }
    case NetworkCheck::PKG_FILE:
    {
        network_impl->pkgFileState = NetworkState::GOOD;
        break;
    }
    }

    Sys_Network_Impl_updateState(network_impl, state_array);
}

#pragma push(pack, 1)
struct Sys_Net_Interface
{
    char pad[8];
    char name[4];     // 8
    uint32_t address; // 12
    uint32_t netmask; // 16
    uint32_t gateway; // 20
    char pad2[12];    // 24
    char mac[4];      // 36
};
#pragma pop(pack)

void(__cdecl *std_string_assign)(void *string, const char *str);

bool jmp_Sys_Net_Interface_update(Sys_Net_Interface *interf)
{
    *(void **)&std_string_assign = (void *)0x80579c8;

    interf->address = bswap32(inet_addr("192.168.92.11"));
    interf->netmask = bswap32(inet_addr("255.255.255.0"));
    interf->gateway = bswap32(inet_addr("192.168.92.254"));
    std_string_assign((void *)interf->name, "eth0");
    std_string_assign((void *)interf->mac, "000000000000");

    return true;
}

uint8_t ourPcb = 255;
uint32_t terminalAddress = 0;

bool isMessagePacket(uint8_t *data)
{
    return data[0] == 0 || data[0] == 1 || data[0] == 2;
}

int(__cdecl *old_bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int jmp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    sockaddr_in *service = (sockaddr_in *)addr;
    if (ntohs(service->sin_port) == 50765)
        service->sin_addr.s_addr = 0;

    return old_bind(sockfd, addr, addrlen);
}

int(__cdecl *old_connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int jmp_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    sockaddr_in *service = (sockaddr_in *)addr;

    // Hacky solution
    if (ntohs(service->sin_port) == 50765)
        service->sin_addr.s_addr = terminalAddress;

    return old_connect(sockfd, addr, addrlen);
}

ssize_t (__cdecl* old_recvmsg)(int fd, struct msghdr *msg, int flags);
ssize_t jmp_recvmsg(int fd, struct msghdr *msg, int flags)
{
    int ret = old_recvmsg(fd, msg, flags);
    if (ret < 0)
        return ret;
    if (msg->msg_name)
    {
        // returning -1 EAGAIN make it not poll again, maybe because we dont implement epoll?
        struct sockaddr_in *in = ((struct sockaddr_in *)msg->msg_name);
        int port = ntohs(in->sin_port);

        uint8_t *data = (uint8_t *)msg->msg_iov->iov_base;
        if (port == 50765 && msg->msg_iov->iov_len >= 2)
        {
            if (isMessagePacket(data))
            {
                if (data[1] != ourPcb)
                {
                    switch (data[1])
                    {
                    case 0:
                    {
                        in->sin_addr.s_addr = inet_addr("192.168.92.11");
                        break;
                    }
                    case 1:
                    {
                        in->sin_addr.s_addr = inet_addr("192.168.92.12");
                        break;
                    }
                    case 2:
                    {
                        in->sin_addr.s_addr = inet_addr("192.168.92.13");
                        break;
                    }
                    case 3:
                    {
                        in->sin_addr.s_addr = inet_addr("192.168.92.14");
                        break;
                    }
                    case 4:
                    {
                        if (!config.terminal)
                            terminalAddress = in->sin_addr.s_addr;
                        in->sin_addr.s_addr = inet_addr("192.168.92.20");
                        break;
                    }
                    }
                }
            }
        }
    }
    return ret;
}

ssize_t (__cdecl* old_sendmsg)(int fd, const struct msghdr *msg, int flags);
ssize_t jmp_sendmsg(int fd, const struct msghdr *msg, int flags)
{
    if (msg->msg_name)
    {
        struct sockaddr_in *in = ((struct sockaddr_in *)msg->msg_name);
        int port = ntohs(in->sin_port);
        if (port == 50765 && msg->msg_iov->iov_len >= 2)
        {
            uint8_t *data = (uint8_t *)msg->msg_iov->iov_base;
            if (isMessagePacket(data))
            {
                ourPcb = data[1];
            }
        }
    }
    return old_sendmsg(fd, msg, flags);
}

int sslCallback(int preverify_ok, void *ctx)
{
    return 1;
}

void(__cdecl *old_SSL_CTX_set_verify)(void *, int, void *);
void jmp_SSL_CTX_set_verify(void *a1, int mode, void *callback)
{
    old_SSL_CTX_set_verify(a1, 0, (void *)sslCallback);
}

typedef void *(*t_TLSv1_method)();
// they tried to load TLSv1_method from here :sob:
t_TLSv1_method TLSv1_method_fake = NULL;

void *jmp_SSLv23_method()
{
    if (!TLSv1_method_fake)
        TLSv1_method_fake = (t_TLSv1_method)0x8059408;
    return TLSv1_method_fake();
}

void net_init()
{
    Line::Hook((void *)0x821c920, (void *)jmp_Sys_Net_interface_isAddressChange);
    Line::Hook((void *)0x80aa7d0, (void *)jmp_Sys_Network_UpdateState);

    Line::Hook((void *)0x821d4f0, (void *)jmp_Sys_Net_Interface_update);

    Line::Hook((void *)0x8059ba8, (void *)jmp_bind, (void **)&old_bind);
    Line::Hook((void *)0x8057bf8, (void *)jmp_connect, (void **)&old_connect);
    Line::Hook((void *)0x8058ca8, (void *)jmp_recvmsg, (void **)&old_recvmsg);
    Line::Hook((void *)0x8058bf8, (void *)jmp_sendmsg, (void **)&old_sendmsg);

    Line::Hook((void *)0x8059628, (void *)jmp_SSL_CTX_set_verify, (void **)&old_SSL_CTX_set_verify);
    Line::Hook((void *)0x8058718, (void *)jmp_SSLv23_method);

    terminalAddress = inet_addr("127.0.0.1");
}