#include "util.h"

#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>

void Daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        std::cout << "Cannot create another process" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    if (!freopen("./webserver.log", "w", stdout)) {
        perror("Failed");
        exit(EXIT_FAILURE);
    }

    pid_t sid = setsid();
    if (sid < 0) {
        perror("Failed to initialize sid");
        exit(EXIT_FAILURE);
    }
    
    if (chdir("/") < 0) {
        perror("Failed chdir");
        exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDERR_FILENO);
}

Params ParseParams(int argc, char** argv) {
    int c;
    Params params;
    while ((c = getopt(argc, argv, "h:p:d:")) != -1) {
        switch (c) {
            case 'h':
            params.host = std::string(optarg);
            break;

            case 'p':
            params.port = atoi(optarg);
            break;

            case 'd':
            params.dir = std::string(optarg);
            break;

            default:
            std::cout << "Unknown cmd parameters " << c << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if ((params.host == "") || (params.dir == "") || (params.port == 0)) {
        std::cout << "Not all parameters set!" << std::endl;
        exit(EXIT_FAILURE);
    }
    return params;
}
