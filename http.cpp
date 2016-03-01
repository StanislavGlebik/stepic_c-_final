#include "http.h"

#include <unistd.h>
#include <sys/socket.h>

#include <stdio.h>
#include <errno.h>

#include <iostream>
#include <thread>
#include <fstream>
#include <streambuf>
#include <sstream>

namespace {

const std::string GET_REQUEST = "GET";

void shutdown_connection(int sock) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

// TODO(stash): move to util?
bool FileExists(const std::string& filename) {
    std::ifstream ifs(filename);
    bool res = ifs.good();
    ifs.close();
    return res;
}

std::string ReadFile(const std::string& filename) {
    std::ifstream ifs(filename);
    return std::string(std::istreambuf_iterator<char>(ifs),
        std::istreambuf_iterator<char>());
}

std::string Generate404() {
    char outbuf[] =
        "HTTP/1.0 404 Not Found\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: 10\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Not found!";

    return std::string(outbuf);
}

std::string Generate200(const std::string& filename) {
    const std::string content = ReadFile(filename);
    std::ostringstream oss;
    oss << 
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: " << content.size() << "\r\n"
        "Connection: close\r\n"
        "\r\n" << content;

    return oss.str();
}

}

void RequestHandler(int sock, const std::string& dir) {
    std::cout << "Accepted" << std::endl;
    char* buf = new char[10000];
    // TODO(stash): better read
    auto res = read(sock, buf, 10000);
    if (-1 == res) {
        perror("Read failed");
        shutdown_connection(sock);
        return;
    } else {
        std::cout << std::string(buf, res) << std::endl;
    }
    
    std::string request(buf, res);
    if (request.substr(0, GET_REQUEST.size()) != GET_REQUEST) {
        return;
    }

    auto offset = GET_REQUEST.size() + 1; 
    auto spacePos = request.find(' ', offset);
    auto path = request.substr(offset, spacePos - offset);
    
    auto cgipos = path.find('?');
    if (cgipos != std::string::npos) {
        path = path.substr(0, cgipos);
    }
    // TODO(stash): security hole! 
    auto fullPath = dir + "/" + path;
   
    std::string result; 
    if (FileExists(fullPath)) {
        std::cout << "Exists!" << std::endl;
        result = Generate200(fullPath);
    } else {
        std::cout << "Not found!" << std::endl;
        result = Generate404();
    }

    write(sock, result.c_str(), result.size());
    shutdown_connection(sock);
    std::cout << "Handled by: " << std::this_thread::get_id() << std::endl;
}

