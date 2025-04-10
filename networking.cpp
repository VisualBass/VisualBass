#include "networking.h"
#include <winsock2.h>
#include <chrono>
#include <cstdio>

void SendToPython(float glow, float hue) {
    static SOCKET udpSocket = INVALID_SOCKET;
    static sockaddr_in serverAddr = {};
    static bool socketInitialized = false;

    // Initialize socket if not already done
    if (!socketInitialized) {
        udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udpSocket == INVALID_SOCKET) {
            printf("UDP socket creation failed: %d\n", WSAGetLastError());
            return;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(7777); // Ensure the port is correct
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Set socket to non-blocking mode
        u_long mode = 1;
        ioctlsocket(udpSocket, FIONBIO, &mode);

        socketInitialized = true;
    }

    // Send data
    char payload[64]; // Payload buffer size should be sufficient for your data
    snprintf(payload, sizeof(payload), "%.3f,%.3f", glow, hue);

    int sent = sendto(udpSocket, payload, (int)strlen(payload), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (sent == SOCKET_ERROR) {
        printf("UDP send failed: %d\n", WSAGetLastError());
    }
}
