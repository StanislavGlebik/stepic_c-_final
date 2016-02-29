#include "queue.h"
#include "util.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <errno.h>
#include <string>
#include <thread>

#define PORT 1234
#define POOL_SIZE 1

const std::string GET_REQUEST = "GET";

void shutdown_connection(int sock) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

void listener_cb(int sock, const std::string& dir) {
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
    
    auto fullPath = dir + "/" + path;

    char outbuf[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: 4\r\n"
        "Connection: close\r\n"
        "\r\n"
        "aaaa";
    write(sock, outbuf, sizeof(outbuf));
    shutdown_connection(sock);
    std::cout << "Handled by: " << std::this_thread::get_id() << std::endl;
}

class WorkerThread {
public:
    WorkerThread(ThreadQueue<int>& q) : q_(q) {
    }
    
    WorkerThread(const WorkerThread& w) : q_(w.q_) {
    }

    void operator()() {
        while (true) {
            q_.Pop();
            std::cout << "Got" << std::endl;
        }
    }

private:
    ThreadQueue<int>& q_;
};

int main(int argc, char** argv) {
    Params params = ParseParams(argc, argv);
    std::cout << "Host: " << params.host << std::endl;
    std::cout << "Port: " << params.port << std::endl;
    std::cout << "Dir: " << params.dir << std::endl;

    Daemonize();

    ThreadQueue<int> t_q;
    // TODO(stash): why cannot pass WorkerThread here (copy constructor error)?
    std::string dir = params.dir;
    std::thread worker([&t_q, dir] ()
        {
            while (true) {
                auto x = t_q.Pop();
                listener_cb(x, dir);
            }
        }
    );

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    int enable = 1;    
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (-1 == bind(sock,(sockaddr*)&sa, sizeof(sockaddr_in))) {
        perror("bind() failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(sock, -1)) {
        perror("listen() failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (true) {
        std::cout << "Before accept" << std::endl;
        auto accepted_sock = accept(sock, NULL, NULL);
        if (-1 == accepted_sock) {
            perror("accept failed");
            close(accepted_sock);
            continue;
        }
        t_q.Push(accepted_sock);
    }
    worker.join();
}
