#pragma once
const int MSG_TYPE_TEST = -1, MSG_TYPE_EXIT = 0, MSG_TYPE_REPLY = 1, MSG_TYPE_REQUEST = 2;
const int MSG_MSG_LEN = 100, RI_MSG_LEN = 100;
const int DP_STATUS_OFF = 0, DP_STATUS_ON = 1;
const int DIR_PATH_LEN=100,FILE_PATH_LEN=100;
struct message
{
    message() = default;
    message(int type, const char *message);
    int type_ = MSG_TYPE_TEST;
    char message_[MSG_MSG_LEN];
};