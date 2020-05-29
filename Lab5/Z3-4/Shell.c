#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER 512
#define DELIMITERS " \t\r\n"

char** splitLine(char *, char *);

void quitHandler(int s)
{
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  printf("\nLsh: %s%s", cwd, getuid()==0 ? "# " : "$ ");
  fflush(STDIN_FILENO);
}

int cd(char** args)
 {
  if(args[1] == NULL)
  {
    fprintf(stderr, "Lsh: cd expected a parameter\n");
  }
  else
  {
    chdir(args[1]);
  }
  return EXIT_SUCCESS;
}

pid_t create(char** args, int in, int out, int err)
{
  pid_t pid;
  int status;

  pid = fork();
  if(pid == 0)
  {
    if(in != STDIN_FILENO)
    {
      //The dup2() system call is similar to dup() but the basic difference
      // between them is that instead of using the lowest-numbered unused
      // file descriptor, it uses the descriptor number specified by the user.
      dup2(in, STDIN_FILENO);
      close(in);
    }

    if(out != STDOUT_FILENO)
    {
      dup2(out, STDOUT_FILENO);
      close(out);
    }

    if(err != STDERR_FILENO)
    {
      dup2(err, STDERR_FILENO);
      close(err);
    }

    if(execvp(args[0], args) == -1)
    {
      fprintf(stderr, "Lsh error = %s\n", args[0]);
    }
    exit(EXIT_FAILURE);
  }
  return pid;
}

int start(char** commands, int n, int run_in_bg, int redirect_type, char* redirect_file)
{
  char** com_args;

  pid_t pid;
  int status = EXIT_SUCCESS;

  int i = 0;

  int fd[2];
  int in = STDIN_FILENO;
  int out = STDOUT_FILENO;
  int err = STDERR_FILENO;
  for (i = 0; i < n - 1; ++i)
  {
    com_args = splitLine(commands[i], DELIMITERS);
    if(pipe(fd))
    {
      return 1;
    }
    pid = create(com_args, in, fd[STDOUT_FILENO], err);
    close(fd[STDOUT_FILENO]);
    in = fd[STDIN_FILENO];
  }
  com_args = splitLine(commands[i], DELIMITERS);
  // Check if not empty
  if (com_args[0] == NULL)
  {
    return EXIT_SUCCESS;
  }

  if (strcmp(com_args[0], "cd") == 0)
  {
    return cd(com_args);
  }

  if (strcmp(com_args[0],"exit") == 0)
  {
    exit(EXIT_SUCCESS);
  }

  // Checks standart input, output, error
  if(redirect_type >= 0)
  {
    switch(redirect_type)
    {
      case STDOUT_FILENO:
      {
        // Create a new file or rewrite an existing one(Permitions for users)
        out = creat(redirect_file, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(out < 0)
        {
          return 1;
        }
        break;
      }
      case STDIN_FILENO:
      {
        in = open(redirect_file, O_RDONLY);
        if(in < 0)
        {
          return 1;
        }
        break;
      }
      case STDERR_FILENO:
      {
        printf("STDERR to %s\n", redirect_file);
        err = creat(redirect_file, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(out < 0)
         {
          return 1;
        }
        break;
      }
    }
  }

  pid = create(com_args, in, out, err);
  // Wait for process
  if(!run_in_bg)
  {
    // WIFEXITED- returns a nonzero value if the child process terminated normally with exit or _exit
    // WIFSIGNALED-returns nonzero for child
    while(!WIFEXITED(status) && !WIFSIGNALED(status));
    {
      // Wait until finished
      waitpid(pid, &status, WUNTRACED);
    }
  }
  return 1;
}

char* readLine()
{
  int bufsize = BUFFER;
  char* buffer = malloc(sizeof(char) * bufsize);
  int c;
  int position = 0;

  if(!buffer)
  {
    exit(EXIT_FAILURE);
  }

  while(1)
  {
    c = getchar();
    if(position == 0 && c == EOF)
    {
      exit(EXIT_SUCCESS);
    }
    else
    {
      if(c == '\n' || c == EOF)
      {
        buffer[position] = '\0';
        return buffer;
      }
      else
      {
        buffer[position] = c;
      }
    }
    position++;
  }
}

char** splitLine(char* line, char* delims)
{
  int bufsize = BUFFER;
  char* token;
  char** tokens = malloc(sizeof(char*) * bufsize);
  char** tokens_temp;
  if(!tokens)
  {
    exit(EXIT_FAILURE);
  }
  token = strtok(line, delims);
  int pos = 0;
  while(token != NULL)
  {
    tokens[pos] = token;
    pos++;
    if(pos >= bufsize)
    {
      tokens_temp = tokens;
      bufsize += BUFFER;
      tokens = realloc(tokens, sizeof(char*) * bufsize);
      if(!tokens)
      {
        free(tokens_temp);
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, delims);
  }
  // Add NULL at the end to signal last element
  tokens[pos] = NULL;
  return tokens;
}

void lsh_loop()
{
  char * line;
  char ** commands;
  int status;
  int comnum;
  int run_in_bg;

  char * redirect_file;
  int redirect_type = -1;

  char * ampersand_pos = NULL;
  char * redirect_pos = NULL;

  while(1)
  {
    redirect_type = -1; // -1 means no redirect
    run_in_bg = 0;
    ampersand_pos = NULL;
    redirect_pos = NULL;

    //Print prompt
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Lsh: %s%s", cwd, getuid()==0 ? "# " : "$ ");

    line = readLine();

    if(ampersand_pos = strchr(line, '&'))
    {
      // Remove ampersand from args and replace it with null byte
      *ampersand_pos = '\0';
      run_in_bg = 1;
    }

    if(redirect_pos = strchr(line, '<'))
    {
      redirect_type = STDIN_FILENO;
      *redirect_pos = '\0';
      redirect_file = redirect_pos+1;
      while(*redirect_file == ' ')
      {
        redirect_file++;
      }
    }
    // Redirect null
    if(redirect_pos = strchr(line, '2'))
    {
      if(*(redirect_pos+1) == '>')
      {
        redirect_type = STDERR_FILENO;
        *redirect_pos = '\0';
        *(redirect_pos+1) = '\0';
        redirect_file = redirect_pos+2;
        while(*redirect_file == ' ')
        {
          redirect_file++;
        }
      }
    }
    else if(redirect_pos = strchr(line, '>'))
    {
      redirect_type = STDOUT_FILENO;
      *redirect_pos = '\0';
      redirect_file = redirect_pos+1;
      while(*redirect_file == ' ')
      {
        redirect_file++;
      }
    }
    commands = splitLine(line, "|");
    for(comnum = 0; commands[comnum] != NULL; comnum++);
    status = start(commands, comnum, run_in_bg, redirect_type, redirect_file);
    free(line);
  }
}

int main()
{
  printf("Welcome to LSH\n");
  // Handle CTRL+C
  signal(SIGINT, quitHandler);
  // Remove zombies
  struct sigaction sigchld_action =
  {
    .sa_handler = SIG_DFL,
    .sa_flags = SA_NOCLDWAIT
  };
  sigaction(SIGCHLD, &sigchld_action, NULL);
  lsh_loop();
  return 0;
}
