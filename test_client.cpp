#include <iostream>
#include <socket>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

void send_fuzzed() {
    std::ifstream file("test_command.xml", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: file test_command.xml not found" << std::endl;
        return;
    }
    
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<unsigned char> data(file_size);
    file.read(reinterpret_cast<char*>(data.data()), file_size);
    file.close();
    
    if (!data.empty()) {
        int pos = rand() % data.size();
        unsigned char old_val = data[pos];
        data[pos] = rand() % 256;
        
        printf("Changed the byte %d: %02x ? %02x\n", pos, old_val, data[pos]);
    }
    
#ifdef _WIN32
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
#endif
    
    if (sock < 0) {
        std::cerr << "Error: couldn't create socket" << std::endl;
        return;
    }
    
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(61499);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: failed to connect to the server" << std::endl;
        return;
    }
    
    send(sock, reinterpret_cast<const char*>(data.data()), data.size(), 0);
    
    char response[1024];
    ssize_t bytes_received = recv(sock, response, sizeof(response) - 1, 0);
    
    if (bytes_received > 0) {
        response[bytes_received] = '\0';
        printf("Answer: %s\n", response);
    } else {
        std::cout << "The server did not respond" << std::endl;
    }
    
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "Winsock initialization error" << std::endl;
        return 1;
    }
#endif
    
    for (int i = 0; i < 100; i++) {
        printf("\n--- Test %d ---\n", i + 1);
        try {
            send_fuzzed();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            break;
        }
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}