#include "networking.h"
#include <winsock2.h>
#include <cstdio>

static SOCKET udpSocket = INVALID_SOCKET;
static sockaddr_in serverAddr;
static bool initialized = false;

void SendToPython(float brightness, float hue) {
    if (!initialized) {
        udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(7777); // Matches lifx_controller.py UDP_PORT
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        initialized = true;
    }

    if (udpSocket != INVALID_SOCKET) {
        char buffer[64];
        int len = sprintf(buffer, "%.3f,%.3f", brightness, hue);
        sendto(udpSocket, buffer, len, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    }
}

void LaunchLIFX() {
    // Launch using the unique window title we can target later
    system("start \"LIFX_CONTROLLER\" /min python lifx_controller.py");
}

void StopLIFX() {
    system("taskkill /FI \"WINDOWTITLE eq LIFX_CONTROLLER*\" /F /T >nul 2>&1");
}