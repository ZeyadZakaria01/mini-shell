/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"
%}
%%

\n 	{
    return NEWLINE;
}

[ \t] 	{
    /* Discard spaces and tabs */
}	

">" 	{
    return GREAT;
}
">>" 	{
    return GGREAT;
}

"<" 	{
    return LESS;
}

"|"     {
    return PIPE;
}

"&"     {
    return AMPERSAND;
}

">&"    {
    return GREATAMPERCAND;
}

"exit"  {
    printf("        Good Bye\n");
    exit(1);
}

">>&"    {
    return GGREATAMPERCAND ;
}
"&>>"    {
    return GGREATAMPERCAND ;
}

[^ \t\n\>\|\&\][^ \t\n]+ {
/* [^ \t\n|][^ \t\n]*  { */
    /* Assume that file names have only alpha chars */
    yylval.string_val = strdup(yytext);
    return WORD;
}

.	{
    return NOTOKEN;
}



%%

// DISCLAIMER
// the exit action can be removed as we already handeled it in the logic of our shell (command.cc)
