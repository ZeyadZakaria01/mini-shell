/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <bits/posix_opt.h>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Create available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)malloc(_numberOfSimpleCommands *
											   sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(
			_simpleCommands,
			_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	pid_t pid;
	int status;

	int defaultin = dup(STDIN_FILENO);	 // Default file Descriptor for stdin
	int defaultout = dup(STDOUT_FILENO); // Default file Descriptor for stdout
	int defaulterr = dup(STDERR_FILENO); // Default file Descriptor for stderr

	int fdin, outfd;

	// handling _inputFile
	if (_inputFile)
	{
		fdin = open(_inputFile, 0);
		if (fdin < 0)
		{
			printf("Could not create file");
			exit(1);
		}
	}

	// declaring our pipe
	int fdpipes[2];

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		SimpleCommand *cmd = _simpleCommands[i];

		pipe(fdpipes);
		if (i != _numberOfSimpleCommands - 1)
		{
			dup2(fdpipes[1], 1);
		}
		else
		{
			if (_outFile)
			{

				// outfd = creat(_outFile, 0600);
				if (_append)
				{
					outfd = open(_outFile, O_APPEND | O_RDWR, 0777);

					/* failure */
					if (outfd < 1)
					{
						outfd = open(_outFile, O_CREAT | O_RDWR, 0777);
					}
				}
				else
				{
					outfd = creat(_outFile, 0777);
				}
				// makes outfd file descriptor = 1
				dup2(outfd, 1);
				close(outfd);
			}
			// default output if no _outFile
			else
			{
				dup2(defaultout, 1);
			}
		}
		close(fdpipes[1]);
		pid = fork();
		if (pid == -1)
		{
			printf("could not fork");
			exit(EXIT_FAILURE);
		}
		// child
		if (pid == 0)
		{
			close(fdpipes[0]);
			close(defaultin);
			close(defaultout);
			close(defaulterr);
			execvp(cmd->_arguments[0], cmd->_arguments);
		}
		dup2(fdpipes[0], 0);
		close(fdpipes[0]);
	}

	// restoring to default input/output/error
	dup2(defaultin, 0);
	close(defaultin);
	if (_inputFile != 0)
	{
		close(fdin);
	}

	dup2(defaultout, 1);
	close(defaultout);
	if (_outFile)
	{
		close(outfd);
	}

	dup2(defaulterr, 2);
	close(defaulterr);

	// handling parent wait
	if (_background == 0)
	{
		waitpid(pid, &status, 0);
	}

	// Clear to prepare for next command
	clear();

	// Print new prompt
	prompt();
}

// Shell implementation

void Command::prompt()
{
	printf("myshell> ");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
