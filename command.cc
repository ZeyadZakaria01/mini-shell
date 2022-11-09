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
#include <bits/stdc++.h>
#include <cstddef>
#include <cstdlib>
#include <fcntl.h>
#include <glob.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"
using namespace std;
string absolute = ".";
string logf_name = "shell.log";
char cwd[512];
void sigchild_handler(int id)
{
  string buffer = "";
  time_t now = time(0);
  char *dateTime = ctime(&now);
  string msg = "Child Termintated ";
  buffer = msg + dateTime + "\n";
  FILE *fp;
  fp = fopen(logf_name.c_str(), "a");
  int retVal = fwrite(buffer.c_str(), buffer.length(), 1, fp);
  wait(nullptr);
  fclose(fp);
}

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
    _arguments = (char **)realloc(_arguments, _numberOfAvailableArguments * sizeof(char *));
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
  _simpleCommands = (SimpleCommand **)malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

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
  _numberOfSimpleCommands = 0;
  _outFile = 0;
  _inputFile = 0;
  _errFile = 0;
  _append = 0;
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

vector<string> glob(const string &pattern)
{
  // glob struct resides on the stack
  glob_t glob_result;
  memset(&glob_result, 0, sizeof(glob_result));

  // do the glob operation
  int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
  if (return_value != 0)
  {
    globfree(&glob_result);
    return {pattern};
  }

  // collect all the filenames into a std::list<std::string>
  vector<string> filenames;
  for (size_t i = 0; i < glob_result.gl_pathc; ++i)
  {
    filenames.push_back(string(glob_result.gl_pathv[i]));
  }

  // cleanup
  globfree(&glob_result);

  // done
  return filenames;
}

void Command::execute()
{
  // Don't do anything if there are no simple commands
  if (_numberOfSimpleCommands == 0)
  {
    prompt();
    return;
  }
  print();

  if (chdir(absolute.c_str()) != 0)
  {
    printf("error in chdir");
  }
  else
  {
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
      printf("error in getting cwd");
    }
  }
  // Print contents of Command data structure
  if (!strcmp(_simpleCommands[0]->_arguments[0], "exit"))
  {
    printf("\t\tGood bye!\n");
    exit(5);
    return;
    // needs to be 1 as we need to indicate
    // to indicate that the process didnt end in a peacful way
  }

  if (!strcmp(_simpleCommands[0]->_arguments[0], "cd"))
  {
    if (_simpleCommands[0]->_numberOfArguments > 1)
      absolute = _simpleCommands[0]->_arguments[1];
    else
      absolute = (getenv("HOME"));

    if (chdir(absolute.c_str()) != 0)
    {
      printf("error in chdir\n");
    }
    else
    {
      if (getcwd(cwd, sizeof(cwd)) == NULL)
      {
        printf("Error in getting cwd\n");
      }
      else
      {
        absolute = cwd;
      }
    }
    clear();
    prompt();
    return;
  }

  pid_t pid;
  int status;

  int defaultin = dup(STDIN_FILENO);   // Default file Descriptor for stdin
  int defaultout = dup(STDOUT_FILENO); // Default file Descriptor for stdout
  int defaulterr = dup(STDERR_FILENO); // Default file Descriptor for stderr

  int fdin, outfd;

  // handling _inputFile
  if (_inputFile)
  {
    fdin = open(_inputFile, 0);
    dup2(fdin, 0);
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
    vector<bool> flags(cmd->_numberOfArguments, false);
    for (int i = 0; i < cmd->_numberOfArguments; i++)
    {
      string str = cmd->_arguments[i];
      size_t found = str.find("*");
      if (found != string::npos)
      {
        flags[i] = true;
      }
    }
    int tmp = cmd->_numberOfArguments;
    for (int j = 1; j < tmp && flags[j]; j++)
    {
      vector<string> vec = glob(cmd->_arguments[j]);
      for (int k = 0; k < vec.size(); k++)
      {
        string s = vec[k];
        char *c = (char *)malloc(s.length() * sizeof(char) + 1);
        strcpy(c, s.c_str());
        if (k == 0)
          cmd->_arguments[j] = c;
        else
          cmd->insertArgument(c);
      }
    }

    if (!strcmp(cmd->_arguments[0], "ls") || !strcmp(cmd->_arguments[0], "grep"))
    {
      string s = "--color=auto";
      char *c = (char *)malloc(s.length() * sizeof(char));
      strcpy(c, s.c_str());
      cmd->insertArgument(c);
    }
    pipe(fdpipes);
    if (i != _numberOfSimpleCommands - 1)
    {
      dup2(fdpipes[1], 1);
    }
    else
    {
      if (_outFile)
      {

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
        if (_errFile)
          dup2(outfd, 2);

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
      int errflg = execvp(cmd->_arguments[0], cmd->_arguments);
      if (errflg == -1)
      {
        cout << "Command not found\n";
        exit(-5);
      }
    }
    else
    {
      signal(SIGCHLD, sigchild_handler);
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

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

void Command::prompt()
{
  cout << "[" << cwd << "]$ ";
  fflush(stdout);
}
void sigintHandler(int sig_num)
{
  printf("\n");
  Command::_currentCommand.prompt();
  signal(SIGINT, sigintHandler);
}

int yyparse(void);

int main()
{
  if (getcwd(cwd, sizeof(cwd)) == NULL)
  {
    printf("error in getting cwd");
    return 1;
  }
  else
  {
    absolute = cwd;
  }
  Command::_currentCommand.prompt();
  signal(SIGINT, sigintHandler);
  yyparse();
  return 0;
}
