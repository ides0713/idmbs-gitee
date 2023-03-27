#include "parse_main.h"
#include "parse.h"
#include "parse_defs.h"
#include <stdio.h>
#include <assert.h>

Re ParseMain::handle(const char *st) {
    int rv = parse(st, query_);
    if (!rv) {
        query_->destroy();
        query_ = nullptr;
        return Re::Fail;
    } else {
        parse_session_ = new ParseSession(nullptr, nullptr, false, query_);
        return Re::Success;
    }
}

Session *ParseMain::callBack() {
    return parse_session_;
}
