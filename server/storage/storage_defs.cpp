#include "storage_defs.h"
#include <string.h>
#include "../../src/common_defs.h"

char* getParentDir(char * current_dir){
    int len=strlen(current_dir);
    if(len==1)
        return current_dir;
    int i;
    for(i=len-1;i>=0 and current_dir[i]!='/';i--);
    return substr(current_dir,0,i-1);
}