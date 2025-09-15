#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "Packet.h"
#include "DnsLayer.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

const int PORT = 9000;

int main() {
    const vector<string> ip_pool = {
        "192.168.1.1", "192.168.1.2", "192.168.1.3", "192.168.1.4", "192.168.1.5",
        "192.168.1.6", "192.168.1.7", "192.168.1.8", "192.168.1.9", "192.168.1.10",
        "192.168.1.11", "192.168.1.12", "192.168.1.13", "192.168.1.14", "192.168.1.15"
    };
    const string DEFAULT_IP = "8.8.8.8";

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "Error: Cannot create socket" << endl;
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Error: Failed to bind socket" << endl;
        return 1;
    }

    cout << "Server listening on port " << PORT << endl;

    while (true) {
        char buffer[2048];
        socklen_t client_len = sizeof(client_addr);

        int bytesReceived = recvfrom(sock, buffer, 2048, 0, (struct sockaddr*)&client_addr, &client_len);

        if (bytesReceived < 8) {
            continue;
        }

        string customHeader(buffer, 8);
        string resolvedIp = DEFAULT_IP;

        try {
            int hour = stoi(customHeader.substr(0, 2));
            int id = stoi(customHeader.substr(6, 2));

            int ip_pool_start = 0;
            const int hash_mod = 5;

            // time-based routing
            if (hour >= 4 && hour <= 11) {
                ip_pool_start = 0;  // morning
            } else if (hour >= 12 && hour <= 19) {
                ip_pool_start = 5;  // afternoon
            } else {
                ip_pool_start = 10; // night
            }

            int offset = id % hash_mod;
            int final_index = ip_pool_start + offset;

            if (final_index >= 0 && final_index < ip_pool.size()) {
                resolvedIp = ip_pool[final_index];
            }

        } catch (const exception& e) {
            // use default IP if header parsing fails
        }

        cout << "Header: " << customHeader << " -> " << resolvedIp << endl;

        sendto(sock, resolvedIp.c_str(), resolvedIp.length(), 0, (struct sockaddr*)&client_addr, client_len);
    }

    close(sock);
    return 0;
}