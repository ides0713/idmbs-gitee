#pragma once
const int MSG_TYPE_TEST = -1, MSG_TYPE_EXIT = 0, MSG_TYPE_REPLY = 1, MSG_TYPE_REQUEST = 2;
const int RI_STATUS_FAIL = -1, RI_STATUS_SUCCESS = 0, RI_STATUS_OTHERFAIL = 13131;
const int MSG_MSG_LEN = 100, RI_MSG_LEN = 100;
const int DP_STATUS_OFF = 0, DP_STATUS_ON = 1;
char *substr(const char *s, int n1, int n2); /*从s中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址*/
struct message
{
    message() = default;
    message(int type, const char *message);
    int type_ = MSG_TYPE_TEST;
    char message_[MSG_MSG_LEN];
};

enum RE
{
    SUCCESS = 0,
    FAIL,
    ERROR
};
