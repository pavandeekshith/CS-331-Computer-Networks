# Compiler
CXX = g++

# Try to detect PcapPlusPlus installation
PCAP_PKG_CONFIG := $(shell pkg-config --exists PcapPlusPlus 2>/dev/null && echo "yes")

ifeq ($(PCAP_PKG_CONFIG),yes)
    # Use pkg-config if available
    CXXFLAGS = -std=c++14 -g $(shell pkg-config --cflags PcapPlusPlus)
    LIBS = $(shell pkg-config --libs PcapPlusPlus)
    LDFLAGS = 
else
    # Manual configuration - use the discovered paths
    CXXFLAGS = -std=c++14 -g -I/usr/local/include/pcapplusplus
    LDFLAGS = -L/usr/lib
    # Use the correct library names found by pkg-config
    LIBS = -lPcap++ -lPacket++ -lCommon++ -lpcap -lpthread
endif

# Target executables
CLIENT = client
SERVER = server

# Default target: build all
all: $(CLIENT) $(SERVER)

# Rule to build the client
$(CLIENT): client.cpp
	@echo "Building client with:"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIBS: $(LIBS)"
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)

# Rule to build the server
$(SERVER): server.cpp
	@echo "Building server with:"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)" 
	@echo "LIBS: $(LIBS)"
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)

# Alternative build using system libpcap only (fallback)
simple: client_simple server_simple

client_simple: client.cpp
	$(CXX) -std=c++14 -g client.cpp -o client_simple -lpcap

server_simple: server.cpp  
	$(CXX) -std=c++14 -g server.cpp -o server_simple -lpcap

# Rule to clean up build files
clean:
	rm -f $(CLIENT) $(SERVER) client_simple server_simple

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build client and server with PcapPlusPlus"
	@echo "  simple    - Build with basic libpcap only"
	@echo "  clean     - Remove all built files"
	@echo "  help      - Show this help"

.PHONY: all simple clean help