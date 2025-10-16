#include "network_ext.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int cmd_netstat(void) {
    printf("=== Network Connections ===\n\n");
    printf("%-8s %-25s %-25s %-12s\n", "Proto", "Local Address", "Foreign Address", "State");
    printf("%-8s %-25s %-25s %-12s\n", "-----", "-------------", "---------------", "-----");
    
    PMIB_TCPTABLE2 pTcpTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    
    pTcpTable = (MIB_TCPTABLE2 *)malloc(sizeof(MIB_TCPTABLE2));
    if (pTcpTable == NULL) return 1;
    
    dwSize = sizeof(MIB_TCPTABLE2);
    if ((dwRetVal = GetTcpTable2(pTcpTable, &dwSize, TRUE)) == ERROR_INSUFFICIENT_BUFFER) {
        free(pTcpTable);
        pTcpTable = (MIB_TCPTABLE2 *)malloc(dwSize);
        if (pTcpTable == NULL) return 1;
    }
    
    if ((dwRetVal = GetTcpTable2(pTcpTable, &dwSize, TRUE)) == NO_ERROR) {
        for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
            char local_addr[50], remote_addr[50];
            struct in_addr local_ip, remote_ip;
            
            local_ip.S_un.S_addr = pTcpTable->table[i].dwLocalAddr;
            remote_ip.S_un.S_addr = pTcpTable->table[i].dwRemoteAddr;
            
            snprintf(local_addr, sizeof(local_addr), "%s:%d", 
                     inet_ntoa(local_ip), ntohs((u_short)pTcpTable->table[i].dwLocalPort));
            snprintf(remote_addr, sizeof(remote_addr), "%s:%d", 
                     inet_ntoa(remote_ip), ntohs((u_short)pTcpTable->table[i].dwRemotePort));
            
            const char *state = "UNKNOWN";
            switch (pTcpTable->table[i].dwState) {
                case MIB_TCP_STATE_LISTEN: state = "LISTEN"; break;
                case MIB_TCP_STATE_ESTAB: state = "ESTABLISHED"; break;
                case MIB_TCP_STATE_TIME_WAIT: state = "TIME_WAIT"; break;
                case MIB_TCP_STATE_CLOSE_WAIT: state = "CLOSE_WAIT"; break;
            }
            
            printf("%-8s %-25s %-25s %-12s\n", "TCP", local_addr, remote_addr, state);
        }
    }
    
    free(pTcpTable);
    return 0;
}

#elif defined(__linux__)
#include <arpa/inet.h>

int cmd_netstat(void) {
    printf("=== Network Connections ===\n\n");
    printf("%-8s %-25s %-25s %-12s\n", "Proto", "Local Address", "Foreign Address", "State");
    printf("%-8s %-25s %-25s %-12s\n", "-----", "-------------", "---------------", "-----");
    
    FILE *fp = fopen("/proc/net/tcp", "r");
    if (!fp) {
        fprintf(stderr, "Cannot open /proc/net/tcp\n");
        return 1;
    }
    
    char line[256];
    fgets(line, sizeof(line), fp); // Skip header
    
    while (fgets(line, sizeof(line), fp)) {
        unsigned int local_addr, local_port, remote_addr, remote_port, state;
        
        if (sscanf(line, "%*d: %X:%X %X:%X %X", 
                   &local_addr, &local_port, &remote_addr, &remote_port, &state) == 5) {
            
            struct in_addr local_ip, remote_ip;
            local_ip.s_addr = local_addr;
            remote_ip.s_addr = remote_addr;
            
            char local_str[50], remote_str[50];
            snprintf(local_str, sizeof(local_str), "%s:%d", inet_ntoa(local_ip), local_port);
            snprintf(remote_str, sizeof(remote_str), "%s:%d", inet_ntoa(remote_ip), remote_port);
            
            const char *state_str = "UNKNOWN";
            switch (state) {
                case 0x01: state_str = "ESTABLISHED"; break;
                case 0x02: state_str = "SYN_SENT"; break;
                case 0x03: state_str = "SYN_RECV"; break;
                case 0x04: state_str = "FIN_WAIT1"; break;
                case 0x05: state_str = "FIN_WAIT2"; break;
                case 0x06: state_str = "TIME_WAIT"; break;
                case 0x07: state_str = "CLOSE"; break;
                case 0x08: state_str = "CLOSE_WAIT"; break;
                case 0x09: state_str = "LAST_ACK"; break;
                case 0x0A: state_str = "LISTEN"; break;
                case 0x0B: state_str = "CLOSING"; break;
            }
            
            printf("%-8s %-25s %-25s %-12s\n", "TCP", local_str, remote_str, state_str);
        }
    }
    
    fclose(fp);
    return 0;
}

#else
int cmd_netstat(void) {
    fprintf(stderr, "Network connections not supported on this platform\n");
    return 1;
}
#endif
