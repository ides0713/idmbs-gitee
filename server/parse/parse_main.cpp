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
    query_ = new QueryInfo();
    query_->SCF_Flag = SCF_ERROR;
    if (return_info_ == nullptr)
    {
        return_info_->set(RI_STATUS_FAIL, "Failed to initialize QueryInfo");
        return return_info_;
    }
    int rv = parse(st, query_);
    if (rv = 0)
        return_info_->set(RI_STATUS_FAIL, "Failed");
    else
        return_info_->set(RI_STATUS_SUCCESS, "Successed");
    return return_info_;
}
