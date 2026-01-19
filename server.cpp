#include "server.h"
#include <iostream>
#include <cstring>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    #define INVALID_SOCKET_CHECK (socket_fd == INVALID_SOCKET)
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET_CHECK (socket_fd < 0)
    #define INVALID_SOCKET -1
#endif

ForteServer::ForteServer(const std::string& host, int port)
    : host(host), port(port) {
    socket_fd = INVALID_SOCKET;
}

ForteServer::~ForteServer() {
    if (socket_fd != INVALID_SOCKET) {
        CLOSE_SOCKET(socket_fd);
    }
}

bool ForteServer::load_boot_file(const std::string& filename) {
    return command_handler.load_boot_file(filename);
}

void ForteServer::handle_client(int client_socket) {
    std::cout << "A new client has joined" << std::endl;
    
    char buffer[4096] = {0};
    
    while (true) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        std::string request(buffer);
        std::cout << "Received: " << request << std::endl;
        
        std::string response = command_handler.process_command(request);
        
        send(client_socket, response.c_str(), response.length(), 0);
        std::cout << "Sent: " << response << std::endl;
        
        std::memset(buffer, 0, sizeof(buffer));
    }
    
    CLOSE_SOCKET(client_socket);
    std::cout << "The client has disconnected" << std::endl;
}

bool ForteServer::start() {
    #ifdef _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }
    #endif
    
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET_CHECK) {
        std::cerr << "Socket creation error" << std::endl;
        return false;
    }
    
    int reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, 
                   (const char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt error" << std::endl;
        CLOSE_SOCKET(socket_fd);
        return false;
    }
    
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    
    if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "bind error" << std::endl;
        CLOSE_SOCKET(socket_fd);
        return false;
    }
    
    if (listen(socket_fd, 5) < 0) {
        std::cerr << "listen error" << std::endl;
        CLOSE_SOCKET(socket_fd);
        return false;
    }
    
    std::cout << "The server is running on " << host << ":" << port << std::endl;
    std::cout << "Waiting for connection..." << std::endl;
    
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        int client_socket = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client_socket < 0) {
            #ifdef _WIN32
                if (WSAGetLastError() != WSAEINTR)
            #else
                if (errno != EINTR)
            #endif
            {
                std::cerr << "The accept error" << std::endl;
            }
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        std::thread client_thread(&ForteServer::handle_client, this, client_socket);
        client_thread.detach();
    }
    
    CLOSE_SOCKET(socket_fd);
    
    #ifdef _WIN32
        WSACleanup();
    #endif
    
    return true;
}