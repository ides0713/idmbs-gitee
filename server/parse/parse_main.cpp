#include "parse_main.h"

void queryInfoInitialize(QueryInfo* ptr){
    ptr = new QueryInfo();
    ptr->SCF_Flag=SCF_ERROR;
}



returnInfo parseMain(const char * st,QueryInfo* res){
    returnInfo return_info;
    queryInfoInitialize(res);
    if (res==nullptr){
        return_info.status_=-1;
        strcpy(return_info.message_,"Failed to initialize QueryInfo");
        return return_info;
    }
    parse(st,res);



    //succcess
    returnInfo
    return 0;
}
