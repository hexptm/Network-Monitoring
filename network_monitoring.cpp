#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define CYAN        "\033[36m"

void getBandwidthUsage() {
    MIB_IFTABLE* ifTable;
    DWORD dwSize = 0;

    GetIfTable(NULL, &dwSize, false);
    ifTable = (MIB_IFTABLE*)malloc(dwSize);

    if (GetIfTable(ifTable, &dwSize, false) == 0) {
        for (DWORD i = 0; i < ifTable->dwNumEntries; ++i) {
            MIB_IFROW ifRow = ifTable->table[i];
            std::cout << CYAN << "Interface " << i + 1 << RESET;
            std::cout << " - Bandwidth: " << YELLOW << ifRow.dwSpeed / 1e6 << " Mbps" << RESET << std::endl;
            std::cout << "InOctets: " << GREEN << ifRow.dwInOctets << RESET;
            std::cout << " OutOctets: " << GREEN << ifRow.dwOutOctets << RESET << "\n";
        }
    }
    free(ifTable);
}

int getLatency(const char* ipAddress) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);  // Port HTTP
    serverAddr.sin_addr.s_addr = inet_addr(ipAddress);

    auto start = std::chrono::high_resolution_clock::now();
    int result = connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));
    auto end = std::chrono::high_resolution_clock::now();
    closesocket(sock);

    if (result == SOCKET_ERROR) {
        std::cerr << RED << "Connection failed.\n" << RESET;
        return -1;
    }

    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return latency;
}

void monitorPacketLoss() {
    int totalPackets = 10;
    int lostPackets = 0;

    for (int i = 0; i < totalPackets; ++i) {
        if (getLatency("8.8.8.8") == -1) {
            ++lostPackets;
        }
        Sleep(500);
    }

    float packetLossPercent = (lostPackets / (float)totalPackets) * 100;
    std::cout << CYAN << "Packet Loss: " << RESET;
    if (packetLossPercent > 0) {
        std::cout << RED << packetLossPercent << "%" << RESET << "\n";
    } else {
        std::cout << GREEN << "0%" << RESET << "\n";
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << RED << "Failed to initialize Winsock.\n" << RESET;
        return 1;
    }

    while (true) {
        std::cout << YELLOW << "===== Network Monitoring Tool =====" << RESET << "\n";
        getBandwidthUsage();
        int latency = getLatency("8.8.8.8");
        if (latency != -1) {
            std::cout << CYAN << "Latency to 8.8.8.8: " << RESET << latency << " ms\n";
        }
        monitorPacketLoss();
        std::cout << YELLOW << "===================================\n\n" << RESET;
        Sleep(5000);
    }

    WSACleanup();
    return 0;
}
