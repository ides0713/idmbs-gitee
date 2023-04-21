#pragma once
#include <vector>
#include <cstring>
#include <string>
#include "base_main.h"

class GlobalMainManager
{
public:
    void init();
    void handle(const char *sql);
    void destroy();
    void setResponse(const char *str);
    void setResponse(std::string str);
    void response();
    void doneResponse();

private:
    std::vector<BaseMain *> mains_;
    std::string response_;

private:
    void clear();
};