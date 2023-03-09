#include "common_defs.h"
#include <string.h>
#include <stdio.h>
message::message(int type, const char *message) : type_(type)
{
    strcpy(message_, message);
}