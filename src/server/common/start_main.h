#pragma once
#include "base_main.h"
#include "../../common/common_defs.h"

extern char *strNew(const char *str);

class StartMain : public BaseMain
{
public:
    StartMain() : sql_(nullptr) { setType(MainType::Start); }
    Re init(BaseMain *last_main) override;
    Re handle() override;
    void clear() override;
    void destroy() override;
    char *getSql();
    void setSql(const char *sql);

private:
    char *sql_;
};