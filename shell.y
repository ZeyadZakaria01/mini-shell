
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE GGREAT LESS LLESS PIPE AMPERSAND GREATAMPERCAND GGREATAMPERCAND;

%union	{
		char   *string_val;
        }

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include <cstring>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
    command
	|commands command 
	;

command: 
       simple_command
       |
        ;

simple_command:	
	pipe_list iomodifier_list background_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
            printf("   Yacc: insert argument \"%s\"\n", $1);
            Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| GGREAT WORD {
        if( Command::_currentCommand._outFile) {
            printf("Already set to another location GGREAT shell.y");
            exit(0);
        }

        printf("out file with append \%s\" \n", $2);
        Command::_currentCommand._outFile = $2;
        Command::_currentCommand._append= 1;
    }
    | GREATAMPERCAND WORD {
            if( Command::_currentCommand._outFile || Command::_currentCommand._errFile) {
                printf("Already set to another location GREATAMPERCAND shell.y");
                exit(0);
            }
            Command::_currentCommand._outFile = $2;
            Command::_currentCommand._errFile = $2;
    }        
    | GGREATAMPERCAND WORD {
            if( Command::_currentCommand._outFile || Command::_currentCommand._errFile) {
                printf("Already set to another location GGREATAMPERCAND shell.y");
                exit(0);
            }
            Command::_currentCommand._outFile = $2;
            Command::_currentCommand._errFile = $2;
            Command::_currentCommand._append = 1;
    }
    | LESS WORD {
		printf("   Yacc: insert input\"%s\"\n", $2);
		Command::_currentCommand._inputFile= $2;
	}
	;

iomodifier_list:
    iomodifier_list iomodifier_opt // it can stack on each others
    | iomodifier_opt
    | // Can be empty
    ;

pipe_list:
    pipe_list PIPE command_and_args
    | command_and_args
    /* can not be empty */
    ;

background_opt:
    AMPERSAND {
        printf("made command run in background");
        Command::_currentCommand._background = 1;
    }
    | /* user wants to run in foreground */
    ;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
