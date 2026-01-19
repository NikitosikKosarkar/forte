#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include "command_handler.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int SocketType;
#else
    typedef int SocketType;
#endif

class ForteServer {
private:
    std::string host;
    int port;
    SocketType socket_fd;
    CommandHandler command_handler;
    
    void handle_client(int client_socket);

public:
    ForteServer(const std::string& host = "localhost", int port = 61499);
    ~ForteServer();
    
    bool load_boot_file(const std::string& filename);
    bool start();
};

#endif
