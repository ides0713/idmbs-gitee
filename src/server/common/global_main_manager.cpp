#include "global_main_manager.h"
#include "start_main.h"
#include "../parse/parse_main.h"
#include "../resolve/resolve_main.h"
#include "../execute/execute_main.h"
#include "../storage/storage_main.h"
void GlobalMainManager::init()
{
    BaseMain *start_main = new StartMain();
    mains_.push_back(start_main);
    BaseMain *parse_main = new ParseMain();
    mains_.push_back(parse_main);
    BaseMain *resolve_main = new ResolveMain();
    mains_.push_back(resolve_main);
    BaseMain *execute_main = new ExecuteMain();
    mains_.push_back(execute_main);
    BaseMain *storage_main = new StorageMain();
    mains_.push_back(storage_main);
}

void GlobalMainManager::handle(const char *sql)
{
    static_cast<StartMain *>(mains_[0])->setSql(sql);
    for (int i = 1; i < mains_.size(); i++)
    {
        Re r = mains_[i]->init(mains_[i - 1]);
        if (r != Re::Success)
        {
            debugPrint("%s main init failed\n", strMainType(mains_[i]->getType()).c_str());
            response();
            for (int j = 0; j <= i; j++)
                mains_[i]->clear();
            break;
        }
        debugPrint("%s main init succeeded\n", strMainType(mains_[i]->getType()).c_str());
        r = mains_[i]->handle();
        if (r != Re::Success)
        {
            debugPrint("%s main handle failed\n", strMainType(mains_[i]->getType()).c_str());
            response();
            for (int j = 0; j <= i; j++)
                mains_[i]->clear();
            break;
        }
        debugPrint("%s main handle succeeded\n", strMainType(mains_[i]->getType()).c_str());
    }
    doneResponse();
}

void GlobalMainManager::destroy()
{
    for (int i = 0; i < mains_.size(); i++)
        mains_[i]->destroy();
    mains_.clear();
}

void GlobalMainManager::setResponse(const char *str)
{
    response_.assign(str);
}

void GlobalMainManager::setResponse(std::string str)
{
    response_.assign(str);
}

void GlobalMainManager::response()
{
    if (response_.empty())
        setResponse("SQL ended and response is empty\n");
    printf("%s", response_.c_str());
}

void GlobalMainManager::doneResponse()
{
    if (response_.empty())
        printf("SQL SUCCEEDED\n");
    else
        response();
}

void GlobalMainManager::clear()
{
    for (int i = 0; i < mains_.size(); i++)
        mains_[i]->clear();
}
