#pragma once
#include "base_main.h"
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>
class GlobalMainManager {
public:
    void Init();
    void Handle(const char *sql);
    void Destroy();
    void SetResponse(std::string str);
    void SetResponse(const char *format, ...);
    void Response();
    void DoneResponse();

private:
    std::vector<BaseMain *> mains_;
    std::string response_;

private:
    void Clear();
};