#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "PcapFileDevice.h"
#include "Packet.h"
#include "DnsLayer.h"
#include "EthLayer.h"
#include "IPv4Layer.h"
#include "UdpLayer.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

const string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 9000;

string createCustomHeader(int packetId) {
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);
    
    stringstream timeStream;
    timeStream << put_time(localtime(&in_time_t), "%H%M%S");

    stringstream idStream;
    idStream << setw(2) << setfill('0') << packetId;

    return timeStream.str() + idStream.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <pcap_file>" << endl;
        return 1;
    }
    string pcapFilePath = argv[1];

    pcpp::IFileReaderDevice* reader = pcpp::PcapFileReaderDevice::getReader(pcapFilePath);
    if (!reader->open()) {
        cerr << "Error: Could not open pcap file '" << pcapFilePath << "'" << endl;
        return 1;
    }

    cout << "| Queried Domain Name | Custom Header Value | Resolved IP Address |" << endl;
    cout << "|---------------------|---------------------|---------------------|" << endl;

    pcpp::RawPacket rawPacket;
    int packetId = 0;

    while (reader->getNextPacket(rawPacket)) {
        pcpp::Packet parsedPacket(&rawPacket);

        pcpp::DnsLayer* dnsLayer = parsedPacket.getLayerOfType<pcpp::DnsLayer>();
        if (dnsLayer == nullptr || dnsLayer->getQueryCount() == 0) {
            continue;
        }
        
        string domainName = dnsLayer->getFirstQuery()->getName();

        // skip local network queries
        if (domainName.find(".local") != string::npos || 
            domainName == "wpad" || domainName == "isatap" || domainName == "isilon") {
            continue;
        }

        if (dnsLayer->getDnsHeader()->queryOrResponse == 0) {
            string customHeader = createCustomHeader(packetId++);

            const uint8_t* originalPacketData = rawPacket.getRawData();
            int originalPacketLen = rawPacket.getRawDataLen();
            
            vector<uint8_t> newPacketBuffer;
            newPacketBuffer.insert(newPacketBuffer.end(), customHeader.begin(), customHeader.end());
            newPacketBuffer.insert(newPacketBuffer.end(), originalPacketData, originalPacketData + originalPacketLen);

            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock < 0) {
                cerr << "Error creating socket" << endl;
                continue;
            }

            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(SERVER_PORT);
            inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr);

            sendto(sock, newPacketBuffer.data(), newPacketBuffer.size(), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

            char responseBuffer[1024];
            int responseLen = recvfrom(sock, responseBuffer, 1024, 0, NULL, NULL);
            responseBuffer[responseLen] = '\0';

            cout << "| " << left << setw(19) << domainName 
                      << " | " << setw(19) << customHeader 
                      << " | " << setw(19) << responseBuffer << " |" << endl;

            close(sock);
        }
    }

    reader->close();
    delete reader;
    
    return 0;
}