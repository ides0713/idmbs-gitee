#pragma once
#include <string.h>
#include <stdio.h>
const int MSG_TYPE_TEST = -1, MSG_TYPE_EXIT = 0, MSG_TYPE_REPLY = 1, MSG_TYPE_REQUEST=2;
const int RI_STATUS_FAIL=-1,RI_STATUS_SUCCESS=0,RI_STATUS_OTHERFAIL=13131;
const int MSG_MSG_LEN=100,RI_MSG_LEN=100;
const int DP_STATUS_OFF=0,DP_STATUS_ON=1;
struct message
{
    message()=default;
    message(int type, const char *message) : type_(type)
    {
        strcpy(message_, message);
    }
    int type_=MSG_TYPE_TEST;
    char message_[MSG_MSG_LEN];
};

enum RE{
    SUCCESS=0,
    FAIL,
    ERROR
};
