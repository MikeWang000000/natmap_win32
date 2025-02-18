/*
    # Using Winsock
    gcc -o wecho.exe socktest.c -DSRV -DWINSOCK -lws2_32
    gcc -o wdns.exe socktest.c -DWINSOCK -lws2_32 
    # Using Posix (MSYS)
    gcc -o pecho.exe socktest.c -DSRV
    gcc -o pdns.exe socktest.c
*/

#define LOCAL_IP "192.168.1.1" /* Modify me! */
#define LOCAL_PORT 11234

#include <stdio.h>
#if defined(WINSOCK)
#include <winsock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
/* equivalents */
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define Sleep(ms) sleep(ms/1000)
#define closesocket(s) close(s)
/* dummies */
#define WSADATA int
#define WSAStartup(x,y) 0
#define WSACleanup(x) 0
#endif

static void puts_hex(const char *s, int recv_len)
{
    while(recv_len--)
        printf("%02x ", (unsigned int) 0xff & (*s++));
    putchar('\n');
    fflush(stdout);
}

int main(void)
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in bindaddr;
    int reuse = 1;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        return 1;

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) == SOCKET_ERROR)
        return 1;

    bindaddr.sin_family = AF_INET;
#ifdef SRV
    bindaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
#else
    bindaddr.sin_addr.s_addr = inet_addr(LOCAL_IP);
#endif
    bindaddr.sin_port = htons(LOCAL_PORT);

    if (bind(s, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) == SOCKET_ERROR)
        return 1;

    printf("(bound to %s:%d)\n", inet_ntoa(bindaddr.sin_addr), ntohs(bindaddr.sin_port));

#ifdef SRV
    for (;;) {
        char buf[2048] = {0};
        struct sockaddr_in client;
        int recv_len, slen = sizeof(client);

        if ((recv_len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client, &slen)) == SOCKET_ERROR)
            return 1;

        printf("(%s:%d) %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buf);

        if (sendto(s, buf, recv_len, 0, (struct sockaddr *)&client, slen) == SOCKET_ERROR)
            return 1;
    }
#else
    {
        int recv_len;
        char dnsreq[] = "\0\0\1\0\0\1\0\0\0\0\0\0\7example\3com\0\0\1\0\1";
        char buf[2048] = {0};
        struct sockaddr_in dnssrv;
        dnssrv.sin_family = AF_INET;
        dnssrv.sin_addr.s_addr = inet_addr("101.6.6.6");
        dnssrv.sin_port = htons(5353);

        if (connect(s, (struct sockaddr *)&dnssrv, sizeof(dnssrv)) == SOCKET_ERROR)
            return 1;

        if (send(s, dnsreq, sizeof(dnsreq)-1, 0) == SOCKET_ERROR)
            return 1;

        if ((recv_len = recv(s, buf, sizeof(buf), 0)) == SOCKET_ERROR)
            return 1;

        printf("(DNS) ");
        puts_hex(buf, recv_len);
        Sleep(1000);
    }
#endif

    closesocket(s);
    WSACleanup();

    return 0;
}
