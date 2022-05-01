#include "stdafx.h"

#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <mstcpip.h>


#pragma comment(lib, "ws2_32.lib")

void ipv6_example()
{
    WSADATA wsd; //WSADATA变量  
    //struct sockaddr_in6 servAddr_in6; //服务器地址 
    //SOCKADDR_IN servAddr; //服务器地址  
    //int retVal; //返回值 
    //int nServAddlen;

    //初始化套结字动态库  
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        return;
    }

    struct addrinfo hints, *ai/*, *aitop*/;
    char strport1[32] = { 0 };
    int gaierr;
    addrinfo *listen_addrs;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC/*AF_INET6*/;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    //hints.ai_flags = AI_PASSIVE;// 指定准备根据getaddrinfo拿到的地址用来bind&&listen

    char buf[256] = "";
    gethostname(buf, 256);
    //得到本机的ipv4和ipv6的地址信息
    if ((gaierr = getaddrinfo("cm.bilibili.com", "2243", &hints, &listen_addrs)) == 0)
    //if ((gaierr = getaddrinfo(buf, "", &hints, &listen_addrs)) == 0)
    //if ((gaierr = getaddrinfo(NULL, strport1, &hints, &listen_addrs)) == 0)
    {
        char strport[32] = { 0 };
        int ret, on = 1;
        char ntop[1024] = { 0 };
        for (ai = listen_addrs; ai; ai = ai->ai_next)
        {
            if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6)
                continue;
            if ((ret = getnameinfo(ai->ai_addr, ai->ai_addrlen,
                                   ntop, sizeof(ntop), strport, sizeof(strport),
                                   NI_NUMERICHOST | NI_NUMERICSERV)) == 0)
            {
                switch (ai->ai_family)
                {
                case AF_INET:
                    printf("ipv4-> %s:%s\n", ntop, strport);
                    break;
                case AF_INET6:
                    printf("ipv6-> [%s]:%s\n", ntop, strport);
                    break;
                default:
                    ASSERT(false);
                    break;
                }
            }

            wchar_t buf[64] = { 0 };
            DWORD buflen = sizeof(buf) / sizeof(*buf);
            if (WSAAddressToStringW(ai->ai_addr, ai->ai_addrlen, NULL, buf, &buflen) == 0)
            {
                continue;;
            }
        }
        freeaddrinfo(listen_addrs);
    }
    else
    {
        DWORD er = GetLastError();
        printf("bad addr or host! %d\n", er);
    }
    WSACleanup();
}