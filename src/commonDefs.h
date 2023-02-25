#pragma once
#include<string.h>
const int MSG_TYPE_TEST = -1, MSG_TYPE_EXIT = 0, MSG_TYPE_REPLY = 1, MSG_TYPE_REQUEST=2;
const int MSG_MSG_LEN=100,RI_MSG_LEN=100;
struct message
{
    message() : type_(-1) {}
    message(int type, const char *message) : type_(type)
    {
        strcpy(message_, message);
    }
    int type_;
    char message_[MSG_MSG_LEN];
};

struct returnInfo
{
    returnInfo():status_(-1){}
    int status_;
    char message_[RI_MSG_LEN]
};
