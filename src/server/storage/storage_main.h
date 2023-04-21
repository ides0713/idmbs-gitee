#pragma once

#include "../common/re.h"
#include "../parse/parse.h"
#include "../common/base_main.h"

class StorageMain : public BaseMain
{
public:
    //    explicit ExecuteMain(Session *resolve_session) : resolve_session_(resolve_session), execute_session_(nullptr) {}
    StorageMain() { setType(MainType::Storage); }
    Re init(BaseMain *last_main) override;
    Re handle() override;
    void clear() override;
    void destroy() override;

private:
};