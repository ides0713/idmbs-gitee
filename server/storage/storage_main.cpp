#include "storage_main.h"
#include "storage_defs.h"
#include <sys/stat.h>
#include <sys/types.h>
RE StorageMain::execute(){
    DIR* bin_dir=findBin();

    printf("N N N N N N \n");
    return RE::FAIL;
}
DIR * StorageMain::findBin(){
    char * parent_dir=new char[100];
    getcwd(parent_dir,100);
}
FILE* StorageMain::findDBFile(DIR* bin_dir){}