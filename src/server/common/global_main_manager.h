#pragma once
#include <string>// for string
#include <vector>// for vector
class BaseMain;
class GlobalMainManager
{
public:
    void Init();
    void Handle(const char *sql);
    void Destroy();
    void SetResponse(std::string str);
    void SetResponse(const char *format, ...);
    void Response();
    bool IsResponseSet();
    void ClearResponse();
private:
    std::vector<BaseMain *> mains_;
    std::string response_;

private:
    void Clear();
};