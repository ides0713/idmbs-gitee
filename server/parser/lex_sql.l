%{

#include<string.h>
#include<stdio.h>
#include "yacc_sql.hpp"

// #if YYDEBUG > 0
// #define debug_printf  printf
// #else
// #define debug_printf(...)
// #endif // YYDEBUG
%}
%option case-insensitive

%%
[0-9]+ return NUMBER;
heat return TOKHEAT;
on|off return STATE;
target return TOKTARGET;
temperature return TOKTEMPERATURE;
n /* ignore end of line */;
[ t]+ /* ignore whitespace */
%%