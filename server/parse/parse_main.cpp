#include "parse_main.h"

Parse::Parse()
{
    query_ = nullptr;
    return_info_ = nullptr;
}
Parse::~Parse()
{
    delete query_;
    delete return_info_;
}
returnInfo *Parse::parseMain(const char *st)
{
    printf("11\n");
    query_ = new QueryInfo();
    printf("12\n");
    query_->SCF_Flag = SCF_ERROR;
    printf("13\n");
    if (return_info_ == nullptr)
    {
        return_info_->set(RI_STATUS_FAIL, "Failed to initialize QueryInfo");
        return return_info_;
    }
    printf("14\n");
    int rv = parse(st, query_);
    printf("15\n");
    if (rv = 0)
        return_info_->set(RI_STATUS_FAIL, "Failed");
    else
        return_info_->set(RI_STATUS_SUCCESS, "Successed");
    printf("16\n");
    return return_info_;
}
