#include "queue.h"
#include "util.h"
#include "http.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <iostream>
#include <errno.h>
#include <string>
#include <thread>

#define POOL_SIZE 10

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
    sigignore(SIGHUP);
    Params params = ParseParams(argc, argv);
    std::cout << "Host: " << params.host << std::endl;
    std::cout << "Port: " << params.port << std::endl;
    std::cout << "Dir: " << params.dir << std::endl;

    Daemonize();

    ThreadQueue<int> t_q;
    // TODO(stash): why cannot pass WorkerThread here (copy constructor error)?
    const std::string& dir = params.dir;
    std::thread worker([&t_q, dir] ()
        {
            while (true) {
                auto x = t_q.Pop();
                RequestHandler(x, dir);
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
    sa.sin_port = htons(params.port);
    sa.sin_addr.s_addr = inet_addr(params.host.c_str());

    if (-1 == bind(sock,(sockaddr*)&sa, sizeof(sockaddr_in))) {
        std::cout << "bind() failed " << errno << std::endl;
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
