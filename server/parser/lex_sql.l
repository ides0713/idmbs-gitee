%{
#include "yacc_sql.hpp"
%}
%%
[0-9]+ return NUMBER;
heat return TOKHEAT;
on|off return STATE;
target return TOKTARGET;
temperature return TOKTEMPERATURE;
n /* ignore end of line */;
[ t]+ /* ignore whitespace */
%%
// void scan_string(const char *str, yyscan_t scanner) {
//   yy_switch_to_buffer(yy_scan_string(str, scanner), scanner);
// }