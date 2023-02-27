#pragma once
#include<string.h>
const int MSG_TYPE_TEST = -1, MSG_TYPE_EXIT = 0, MSG_TYPE_REPLY = 1, MSG_TYPE_REQUEST=2;
const int RI_STATUS_FAIL=-1,RI_STATUS_SUCCESS=0,RI_STATUS_OTHERFAIL=13131;
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
    returnInfo():status_(RI_STATUS_FAIL){}
    void set(int status,const char* message){
        status_=status;
        strcpy(message_,message);
    }
    int status_;
    char message_[RI_MSG_LEN];
};
