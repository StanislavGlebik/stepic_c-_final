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

void listener_cb(evconnlistener* listener,
    evutil_socket_t sock, sockaddr* addr, int len, void* ptr) {
    std::cout << "Accepted" << std::endl;
    char* buf = new char[10000];
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
    auto base = event_base_new();
    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    auto listener = evconnlistener_new_bind(base,
        listener_cb /*callback*/,
        NULL /*arg*/,
        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_LEAVE_SOCKETS_BLOCKING, // TODO(stash): Maybe make non-blocking?
        -1,
        (sockaddr*)&sa,
        sizeof(sockaddr_in)
        );

    if (!listener) {
        perror("Listener failed");
        return 1;
    }

    event_base_dispatch(base);
    event_base_free(base);
}
