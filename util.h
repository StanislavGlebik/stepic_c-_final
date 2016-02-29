#pragma once

#include <string>

void Daemonize();

struct Params {
    std::string host;
    int port;
    std::string dir;
};

Params ParseParams(int argc, char** argv);
