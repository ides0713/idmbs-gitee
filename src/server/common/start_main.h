#pragma once
#include "../../common/common_defs.h"
#include "../parse/parse_defs.h"
#include "base_main.h"

class StartMain : public BaseMain {
public:
    StartMain() : sql_(nullptr) { SetType(MainType::Start); }
    Re Init(BaseMain *last_main) override;
    Re Handle() override;
    void Clear() override;
    void Destroy() override;
    char *GetSql();
    void SetSql(const char *sql);

private:
    char *sql_;
};