# DNS Client-Server System

A custom DNS client-server implementation that processes DNS queries from PCAP files and routes them through a time-based load balancing server.

## Overview

This project consists of two main components:
- **Client**: Reads DNS queries from PCAP files and forwards them to the server with custom headers
- **Server**: Receives queries and returns IP addresses based on time-based routing rules

## Prerequisites

### System Requirements
- Linux/Unix system
- C++14 compatible compiler (g++)
- PcapPlusPlus library
- libpcap development headers

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libpcap-dev git cmake build-essential
```

**RedHat/CentOS:**
```bash
sudo yum install libpcap-devel git cmake gcc-c++ make
```

### Installing PcapPlusPlus

If PcapPlusPlus is not installed, you can install it using the provided script:
```bash
chmod +x install_pcapplusplus.sh
./install_pcapplusplus.sh
sudo ldconfig
```

## Building the Project

1. Clone or download the project files
2. Navigate to the project directory
3. Build using make:

```bash
make clean
make
```

## Usage

### Running the Server

Start the server first in one terminal:
```bash
./server
```

The server will listen on port 9000 and display incoming requests.

### Running the Client

In another terminal, run the client with a PCAP file:
```bash
./client <pcap_file>
```

Example:
```bash
./client dns_queries.pcap
```

## How It Works

### Client Process
1. Reads DNS query packets from the provided PCAP file
2. Filters out local network queries (.local, wpad, isatap, etc.)
3. Creates 8-byte custom headers in format "HHMMSSID"
   - HH: Current hour (00-23)
   - MM: Current minute (00-59)
   - SS: Current second (00-59)
   - ID: Packet sequence number (00-99)
4. Sends queries to the server and displays results

### Server Logic
The server implements time-based IP routing:

**Time Periods:**
- Morning (04:00-11:59): Uses IP pool indices 0-4
- Afternoon (12:00-19:59): Uses IP pool indices 5-9
- Night (20:00-03:59): Uses IP pool indices 10-14

**IP Selection:**
- Final IP = ip_pool[time_period_start + (packet_id % 5)]

### IP Pool
The server uses these predefined IPs:
```
192.168.1.1  through 192.168.1.15
```
Fallback IP: `8.8.8.8`

## Output Format

The client displays results in a table format:
```
| Queried Domain Name | Custom Header Value | Resolved IP Address |
|---------------------|---------------------|---------------------|
| example.com.        | 14302301            | 192.168.1.6         |
```

## Project Files

- `client.cpp` - DNS client implementation
- `server.cpp` - DNS server with time-based routing
- `Makefile` - Build configuration
- `find_pcap_libs.sh` - Library detection script
- `install_pcapplusplus.sh` - PcapPlusPlus installation script
- `server_simple.cpp` - Simplified server version

## Troubleshooting

### Build Issues
- Ensure PcapPlusPlus is properly installed
- Try building with `make simple` for basic functionality
- Check library paths with `./find_pcap_libs.sh`

### Runtime Issues
- Ensure the server is running before starting the client
- Check that port 9000 is available
- Verify PCAP file contains valid DNS queries

### Common Errors
1. **"Cannot find -lPcap++"**: PcapPlusPlus not installed correctly
2. **"Error creating socket"**: Permission issues or port conflicts
3. **"Could not open pcap file"**: Invalid file path or corrupted PCAP

## Testing

To test the system:
1. Start the server: `./server`
2. Use a sample PCAP file with DNS queries
3. Run the client: `./client sample.pcap`
4. Observe the time-based IP assignment based on current time

## License

This project is for educational purposes.
