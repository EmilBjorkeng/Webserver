#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <iomanip>

#include "include/server.h"
#include "include/request.h"
#include "include/logger.h"

int main() {
    // DEBUG: Clear the log file at the start of the program
    std::ofstream ofs("debug.log", std::ios::trunc);
    ofs.close();

    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return EXIT_FAILURE;
    }

    // Prepare the server address
    sockaddr_in server_address;
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Set socket options to allow address reuse
    // To prevent "Failed to bind socket" error when restarting the server quickly
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        return EXIT_FAILURE;
    }

    // Bind the socket to the address
    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        return EXIT_FAILURE;
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    HttpRequestHandler requestHandler;

    Logger::getInstance().setOutputFile("debug.log");
    Logger::getInstance().log("Server started", LogLevel::INFO);

    while (true) {
        Logger::getInstance().space();

        // Accept an incoming connection
        sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            Logger::getInstance().log("Failed to accept connection", LogLevel::ERROR);
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        Logger::getInstance().log("Client Socket " + std::to_string(client_socket), LogLevel::INFO);

        // Set the session ID for the current connection
        std::ostringstream sessionID;
        sessionID << "C" << std::setfill('0') << std::setw(3) << std::to_string(client_socket);
        Logger::getInstance().setSessionID(sessionID.str());

        std::string client_info = std::string(inet_ntoa(client_address.sin_addr)) + ":" + std::to_string(ntohs(client_address.sin_port));
        Logger::getInstance().log("Accepted connection from " + client_info, LogLevel::INFO);
        std::cout << "Accepted connection from " << client_info << std::endl;

        // Receive the HTTP request
        char buffer[BUFFER_SIZE];
        std::memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received == -1) {
            std::cerr << "Failed to receive request" << std::endl;
            Logger::getInstance().log("Failed to receive request from " + client_info, LogLevel::ERROR);
            close(client_socket);
            continue;
        }
        else if (bytes_received == -1) {
            std::cerr << "Client disconnected before sending data" << std::endl;
            Logger::getInstance().log("Client disconnected before sending data: " + client_info, LogLevel::INFO);
            close(client_socket);
            continue;
        }

        // Check for TLS handshake (HTTPS attempt)
        if (static_cast<unsigned char>(buffer[0]) == 0x16 &&
            static_cast<unsigned char>(buffer[1]) == 0x03) {
            Logger::getInstance().log("Rejected HTTPS connection attempt from " + client_info, LogLevel::WARNING);
            std::cerr << "Client attempted HTTPS connection. Request rejected" << std::endl;
            close(client_socket);
            continue;
        }

        // Handle the request
        requestHandler.processRequest(client_socket, buffer, bytes_received);

        // Close the client socket
        close(client_socket);
        Logger::getInstance().log("Closed connection with " + client_info, LogLevel::INFO);
        Logger::getInstance().setSessionID("");
    }

    close(server_socket);
    Logger::getInstance().log("Server socket closed", LogLevel::INFO);
    return 0;
}

