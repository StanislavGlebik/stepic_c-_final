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

#define PORT 1234

void listener_cb(evconnlistener* listener,
    evutil_socket_t sock, sockaddr* addr, int len, void* ptr) {
    std::cout << "Accepted" << std::endl;
}

int main() {
    auto base = event_base_new();
    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    // inet_pton(AF_INET, "127.0.0.1", &sa);

    auto listener = evconnlistener_new_bind(base,
        listener_cb /*callback*/,
        NULL /*arg*/,
        LEV_OPT_CLOSE_ON_FREE, // Maybe make blocking?
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
