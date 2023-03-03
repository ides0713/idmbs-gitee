#include "parse_main.h"

Parse::Parse()
{
    query_ = nullptr;
    return_info_ = nullptr;
}
Parse::~Parse()
{
    if(query_!=nullptr)
        query_->destroy();
    delete return_info_;
}
returnInfo *Parse::parseMain(const char *st)
{
    return_info_=new returnInfo();
    if (return_info_ == nullptr)
    {
        return_info_->set(RI_STATUS_FAIL, "Failed to initialize QueryInfo");
        return return_info_;
    }
    int rv = parse(st, query_);
    if (!rv)
        return_info_->set(RI_STATUS_FAIL, "Failed");
    else
        return_info_->set(RI_STATUS_SUCCESS, "Succeeded");
    return return_info_;
}
