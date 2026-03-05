#include <iostream>
#include <arpa/inet.h>
#include <sstream>
#include <map>
#include <algorithm>

#include "include/server.hpp"
#include "include/request.hpp"
#include "include/logger.hpp"

void HttpRequestHandler::processRequest(int client_socket, char *buffer, ssize_t bytes_received) {
    if (bytes_received >= BUFFER_SIZE - 1) {
        Logger::getInstance().log("Buffer overflow detected", LogLevel::ERROR);
        std::cerr << "Buffer overflow detected" << std::endl;
        return;
    }

    std::string request(buffer, bytes_received);

    // Malformed or empty request
    auto bad_request = [&]() {
        Logger::getInstance().log("Malformed or empty request", LogLevel::WARNING);
        std::cerr << "Malformed or empty request" << std::endl;

        std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<html><body><h1>400 Bad Request</h1></body></html>";
        send(client_socket, response.c_str(), response.length(), 0);
    };

    // Parse the request
    std::size_t first_space = request.find(' ');
    if (first_space == std::string::npos) { bad_request(); return; }

    std::size_t second_space = request.find(' ', first_space + 1);
    if (second_space == std::string::npos) { bad_request(); return; }

    std::size_t line_end = request.find("\r\n", second_space + 1);
    if (line_end == std::string::npos) { bad_request(); return; }

    std::size_t body_gap = request.find("\r\n\r\n");
    if (body_gap == std::string::npos) { bad_request(); return; }

    std::string method = request.substr(0, first_space);
    std::string path = request.substr(first_space + 1, second_space - first_space - 1);
    std::string version = request.substr(second_space + 1, line_end - second_space - 1);
    std::string head = request.substr(line_end, body_gap - line_end);
    std::string body = request.substr(body_gap + 4, request.length() - body_gap - 4);

    // Map header keys and values
    std::map<std::string, std::string> header;
    std::replace(head.begin(), head.end(), '\r', ' ');      // Remove Carriage Return
    std::replace(head.begin(), head.end(), (char)32, ' ');  // Remove Record Separator
    std::stringstream headerStream(head);
    std::string line;
    while (std::getline(headerStream, line, '\n')) {
        if (line.empty())
        { continue; }

        std::size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            header[key] = value;
        }
    }

    Logger::getInstance().log("Received " + method + " request for " + path + ", " + version, LogLevel::INFO);
    std::cout << "Received " << method << " request for " << path << ", " << version << std::endl;
    if (!body.empty()) {
        Logger::getInstance().log("Body: " + body, LogLevel::DEBUG);
        std::cout << "Body: " << body << std::endl;
    }
    else {
        Logger::getInstance().log("No body in request", LogLevel::DEBUG);
    }

    // Handle the request
    if (method == "GET")
        handleGET(client_socket, path);
    else if (method == "POST")
        handlePOST(client_socket, path, body);
    else {
        Logger::getInstance().log("Method not implemented: " + method, LogLevel::WARNING);
        std::cout << "Method not implemented: " << method << std::endl;
        sendNotImplemented(client_socket);
    }
    return;
}

// 501 Not Implemented
void HttpRequestHandler::sendNotImplemented(int client_socket) {
    std::string response = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\n\r\n<html><body><h1>501 Not Implemented</h1></body></html>";
    send(client_socket, response.c_str(), response.length(), 0);
    return;
}

// 404 Not Found
void HttpRequestHandler::sendNotFound(int client_socket, const std::string &path) {
    Logger::getInstance().log("Request processed for " + path + ", Status: 404", LogLevel::INFO);
    std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>";
    send(client_socket, response.c_str(), response.length(), 0);
}

// GET Request
void HttpRequestHandler::handleGET(int client_socket, const std::string &path) {
    std::string file_path = "." + path;

    // Default to index.html if no specific file is requested
    if (file_path == "./")
        file_path = "./index.html";

    // Multiple slashes handling
    else if (file_path[file_path.length() - 1] == '/') {
        sendNotFound(client_socket, path);
        return;
    }

    // Serve the requested file
    std::ifstream file(file_path, std::ios::binary);
    if (file) {
        Logger::getInstance().log("Request processed for " + path + ", Status: 200", LogLevel::INFO);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        response += std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        send(client_socket, response.c_str(), response.length(), 0);
        return;
    }
    // 404 Not Found
    else {
        sendNotFound(client_socket, path);
        return;
    }
}

// POST Request
void HttpRequestHandler::handlePOST(int client_socket, const std::string &path, const std::string &body) {
    Logger::getInstance().log("Request processed for " + path + ", Status: 200", LogLevel::INFO);
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + body;
    send(client_socket, response.c_str(), response.length(), 0);
    return;
}

