#include <event2/listener.h>
#include <event2/event.h>
#include <arpa/inet.h>

#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include <string>

#define PORT 1234

void shutdown_connection(evutil_socket_t sock) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

void listener_cb(evutil_socket_t sock) {
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

    char outbuf[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: 4\r\n"
        "Connection: close\r\n"
        "\r\n"
        "aaaa";
    write(sock, outbuf, sizeof(outbuf));
    shutdown_connection(sock);
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket failed");
        close(sock);
        return 1;
    }

    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (-1 == bind(sock,(sockaddr*)&sa, sizeof(sockaddr_in))) {
        perror("bind() failed");
        close(sock);
        return 1;
    }
    
    if (-1 == listen(sock, -1)) {
        perror("listen() failed");
        close(sock);
        return 1;
    }
    
    while (true) {
        auto accepted_sock = accept(sock, NULL, NULL);
        if (-1 == accepted_sock) {
            perror("accept failed");
            close(accepted_sock);
            continue;
        }

        listener_cb(accepted_sock);        
    }
}
