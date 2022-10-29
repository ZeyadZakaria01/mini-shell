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

SimpleCommand::SimpleCommand() {
  // Create available space for 5 arguments
  _numberOfAvailableArguments = 5;
  _numberOfArguments = 0;
  _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument) {
  if (_numberOfAvailableArguments == _numberOfArguments + 1) {
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

Command::Command() {
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

void Command::insertSimpleCommand(SimpleCommand *simpleCommand) {
  if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands) {
    _numberOfAvailableSimpleCommands *= 2;
    _simpleCommands = (SimpleCommand **)realloc(
        _simpleCommands,
        _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
  }

  _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
  _numberOfSimpleCommands++;
}

void Command::clear() {
  for (int i = 0; i < _numberOfSimpleCommands; i++) {
    for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++) {
      free(_simpleCommands[i]->_arguments[j]);
    }

    free(_simpleCommands[i]->_arguments);
    free(_simpleCommands[i]);
  }

  if (_outFile) {
    free(_outFile);
  }

  if (_inputFile) {
    free(_inputFile);
  }

  if (_errFile) {
    free(_errFile);
  }

  _numberOfSimpleCommands = 0;
  _outFile = 0;
  _inputFile = 0;
  _errFile = 0;
  _background = 0;
}

void Command::print() {
  printf("\n\n");
  printf("              COMMAND TABLE                \n");
  printf("\n");
  printf("  #   Simple Commands\n");
  printf("  --- ----------------------------------------------------------\n");

  for (int i = 0; i < _numberOfSimpleCommands; i++) {
    printf("  %-3d ", i);
    for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++) {
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

void Command::execute() {
  // Don't do anything if there are no simple commands
  if (_numberOfSimpleCommands == 0) {
    prompt();
    return;
  }

  // Print contents of Command data structure
  print();

  pid_t pid;
  int status;

  int defaultin = dup(STDIN_FILENO);   // Default file Descriptor for stdin
  int defaultout = dup(STDOUT_FILENO); // Default file Descriptor for stdout
  int defaulterr = dup(STDERR_FILENO); // Default file Descriptor for stderr

  int fdin, outfd;

  if (_inputFile) {
    /* fdin = creat(_inputFile, 0664); */
    fdin = open(_inputFile, O_RDONLY);
    if (fdin < 0) {
      printf("Could not create file");
      exit(1);
    }
    // makes outdf file descriptor = 0
    fdin = dup2(fdin, STDIN_FILENO);
  }
  if (_outFile) {
    outfd = creat(_outFile, 0600);
    if (outfd < 0) {
      printf("Could not create file");
      exit(1);
    }
    // makes outdf file descriptor = 1
    /* dup2(outfd,2); */
    outfd = dup2(outfd, STDOUT_FILENO);
  } 

  /* dup2(defaultin, 0); */

  // Redirect output to pipe (write the output to pipefile[1] instead od
  // stdout)

  // Redirect err (use stderr)
  for (int i = 0; i < _numberOfSimpleCommands - 1; i++) {

    SimpleCommand *cmd = _simpleCommands[i];
    pid = fork();
    if (pid == -1) {
      printf("failed at %s\n", cmd->_arguments[0]);
      exit(EXIT_FAILURE);
    }
    if (pid == 0) { // child
      execvp(cmd->_arguments[0], cmd->_arguments);
    } else {
      if (_background == 0) {
        waitpid(pid, &status, 0);
      }
    }
  }

  // restore output to the previous state
  dup2(defaultout, STDOUT_FILENO);
  dup2(defaultin, STDIN_FILENO);
  dup2(defaulterr,STDERR_FILENO);

  /* close(fdin); */
  /* close(outfd); */

  // Clear to prepare for next command
  clear();

  // Print new prompt
  prompt();
}

// Shell implementation

void Command::prompt() {
  printf("myshell> ");
  fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main() {
  Command::_currentCommand.prompt();
  yyparse();
  return 0;
}
