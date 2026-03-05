#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <unistd.h>
#include <string>

class HttpRequestHandler {
public:
    void processRequest(int client_socket, char* buffer, ssize_t bytes_received);
private:
    void sendNotImplemented(int client_socket);
    void sendNotFound(int client_socket, const std::string &path);
    void handleGET(int client_socket, const std::string& path);
    void handlePOST(int client_socket, const std::string& path, const std::string& body);
};

#endif

