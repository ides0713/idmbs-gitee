#include "parse_main.h"

void queryInfoInitialize(QueryInfo* ptr){
    ptr = new QueryInfo();
    ptr->SCF_Flag=SCF_ERROR;
}



returnInfo parseMain(const char * st,QueryInfo* res){
    returnInfo return_info;
    queryInfoInitialize(res);
    if (res==nullptr){
        return_info.set(RI_STATUS_FAIL,"Failed to initialize QueryInfo");
        return return_info;
    }
    int rv=parse(st,res);
    if(rv=0){

    }else if(rv=1){
        
    }
    //succcess
    return_info.set(RI_STATUS_SUCCESS,"Successed");
    return return_info;
}
