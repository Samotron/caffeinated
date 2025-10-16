#include "nettools.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

int cmd_ping(const char *host) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
    
    struct addrinfo hints, *result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    
    if (getaddrinfo(host, NULL, &hints, &result) != 0) {
        fprintf(stderr, "Could not resolve host: %s\n", host);
        WSACleanup();
        return 1;
    }
    
    struct sockaddr_in *addr = (struct sockaddr_in *)result->ai_addr;
    unsigned long ipaddr = addr->sin_addr.S_un.S_addr;
    
    printf("Pinging %s...\n", host);
    
    HANDLE hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "IcmpCreateFile failed\n");
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    
    char SendData[32] = "Caffeinated ping";
    DWORD ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    LPVOID ReplyBuffer = malloc(ReplySize);
    
    for (int i = 0; i < 4; i++) {
        DWORD dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData),
                                      NULL, ReplyBuffer, ReplySize, 1000);
        
        if (dwRetVal != 0) {
            PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
            printf("Reply from %s: time=%lums\n", host, pEchoReply->RoundTripTime);
        } else {
            printf("Request timed out\n");
        }
        Sleep(1000);
    }
    
    free(ReplyBuffer);
    IcmpCloseHandle(hIcmpFile);
    freeaddrinfo(result);
    WSACleanup();
    return 0;
}

int cmd_portcheck(int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\n");
        WSACleanup();
        return 1;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        printf("Port %d is available (not in use)\n", port);
        closesocket(sock);
        WSACleanup();
        return 0;
    } else {
        printf("Port %d is in use\n", port);
        closesocket(sock);
        WSACleanup();
        return 1;
    }
}

#elif defined(__linux__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <errno.h>

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;
    
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int cmd_ping(const char *host) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    
    if (getaddrinfo(host, NULL, &hints, &result) != 0) {
        fprintf(stderr, "Could not resolve host: %s\n", host);
        return 1;
    }
    
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        fprintf(stderr, "Failed to create socket (need root/sudo for ICMP)\n");
        fprintf(stderr, "Try: sudo ./caffeinated ping %s\n", host);
        freeaddrinfo(result);
        return 1;
    }
    
    struct sockaddr_in *addr = (struct sockaddr_in *)result->ai_addr;
    printf("Pinging %s [%s]...\n", host, inet_ntoa(addr->sin_addr));
    
    for (int i = 0; i < 4; i++) {
        struct icmp icmp_hdr;
        char packet[64];
        
        memset(&icmp_hdr, 0, sizeof(icmp_hdr));
        icmp_hdr.icmp_type = ICMP_ECHO;
        icmp_hdr.icmp_code = 0;
        icmp_hdr.icmp_id = getpid();
        icmp_hdr.icmp_seq = i;
        icmp_hdr.icmp_cksum = 0;
        icmp_hdr.icmp_cksum = checksum(&icmp_hdr, sizeof(icmp_hdr));
        
        memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));
        
        struct timeval start, end;
        gettimeofday(&start, NULL);
        
        if (sendto(sock, packet, sizeof(icmp_hdr), 0, (struct sockaddr *)addr, sizeof(*addr)) <= 0) {
            fprintf(stderr, "Send failed\n");
            continue;
        }
        
        char recv_buf[1024];
        struct sockaddr_in recv_addr;
        socklen_t addr_len = sizeof(recv_addr);
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        int recv_len = recvfrom(sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&recv_addr, &addr_len);
        gettimeofday(&end, NULL);
        
        if (recv_len > 0) {
            long ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
            printf("Reply from %s: time=%ldms\n", host, ms);
        } else {
            printf("Request timed out\n");
        }
        
        sleep(1);
    }
    
    close(sock);
    freeaddrinfo(result);
    return 0;
}

int cmd_portcheck(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Socket creation failed\n");
        return 1;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        printf("Port %d is available (not in use)\n", port);
        close(sock);
        return 0;
    } else {
        if (errno == EADDRINUSE) {
            printf("Port %d is in use\n", port);
        } else {
            printf("Port %d check failed: %s\n", port, strerror(errno));
        }
        close(sock);
        return 1;
    }
}

#else
int cmd_ping(const char *host) {
    fprintf(stderr, "Ping not supported on this platform\n");
    return 1;
}

int cmd_portcheck(int port) {
    fprintf(stderr, "Port check not supported on this platform\n");
    return 1;
}
#endif
