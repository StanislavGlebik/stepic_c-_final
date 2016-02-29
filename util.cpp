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
