#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

void KlientLoop ();
void Login ();
int Execute(char **args);
char *ReadLine();
char **SplitLine(char*);

char directory[PATH_MAX];
int serverSocket;

int Term_cd(char** args);
int Term_ls(char** args);
int Term_pwd(char** args);
int Term_lcd(char** args);
int Term_lls(char** args);
int Term_lpwd(char** args);
int Term_get(char** args);
int Term_put(char** args);
int Term_exit(char** args);

char *builtin_str[] = {
        "cd",
        "ls",
        "pwd",
        "lcd",
        "lls",
        "lpwd",
        "get",
        "put",
        "exit"};
int (*builtin_func[]) (char **) = {
        &Term_cd,
        &Term_ls,
        &Term_pwd,
        &Term_lcd,
        &Term_lls,
        &Term_lpwd,
        &Term_get,
        &Term_put,
        &Term_exit };

int main ()
{
    struct sockaddr_in serverAddr;

    //// socket() //
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == -1) {
        perror("socket : ");
        printf("fail to socket.\n");
        exit (1);
    }
    ////

    //// sockaddr_in //
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(7621);
    ////

    //// connect() //
    if(connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connect : ");
        close(serverSocket);
        exit (1);
    }
    ////

    Login();

    KlientLoop();
    return 0;
}

void KlientLoop () {

    int status;
    char *line;
    char **args;

    getcwd(directory,sizeof(directory));

    do {
        printf("> ");
        line = ReadLine();
        args = SplitLine(line);
        status = Execute(args);

        free(line);
        free(args);
    } while (status);
}

int Execute(char **args)
{

    for (int i = 0; i < 9; i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return 1;
}

char *ReadLine()
{
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

char **SplitLine(char *line)
{

    int bufsize = 64, num = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(line, " \t\r\n\a");

    while (token != NULL) {

        tokens[num] = token;
        num++;
        token = strtok(NULL, " \t\r\n\a");
    }

    tokens[num] = NULL;
    return tokens;
}

void Login () {
    char *login = malloc(sizeof(char)*256);
    char *password = malloc(sizeof(char)*256);
    char *answer = malloc(sizeof(char)*256);

    while(1)
    {
        printf("login: ");
        login = ReadLine();

        printf("password: ");
        password = ReadLine();

        send(serverSocket, login, sizeof(char)*256, 0);
        send(serverSocket, password, sizeof(char)*256, 0);

        recv(serverSocket, answer, sizeof(char)*256, 0);
        printf("server answer: %s\n", answer);
        if(strcmp(answer, "0") == 0) {
            printf("Successful login\n\n");
            return;
        }
        else
            printf("Bad login or password\n\n");
    }
}

int Term_cd(char** args) {

    char* command = malloc(sizeof(char)*256);
    char* buffer = malloc(sizeof(char)*256);

    if (args[1] == NULL) {
        perror( "lsh: expected argument to \"cd\"\n");
    } else {
        sprintf(command, "%s %s\0", args[0], args[1]);
        send(serverSocket, command, sizeof(char)*256, 0);
        recv(serverSocket, buffer,sizeof(char)*256, 0);
    }
    if (buffer == "No such file or directory") {
        printf("No such file or directory\n");
    } else {
        printf("%s\n", buffer);
    }
    return 1;
 }

int Term_ls(char** args) {

    char *buffer =malloc(sizeof(char)*256);
    char *state =malloc( strlen("1"));
    send(serverSocket, args[0], sizeof(char)*256, 0);
    recv(serverSocket, state,strlen("1"), 0);

    send(serverSocket, " ", strlen("1"), 0);
    while(strcmp(state,"1")==0)
    {
        recv(serverSocket,buffer,sizeof(char)*256 , 0);
        printf("%s\n",buffer);
        send(serverSocket, " ", strlen("1"), 0);
        recv(serverSocket, state,strlen("1"), 0);
        send(serverSocket, " ", strlen("1"), 0);
    }
    return 1;
 }

int Term_pwd(char** args) {

    char *buffer =malloc(sizeof(char)*256);
    send(serverSocket, args[0], sizeof(char)*256, 0);
    recv(serverSocket, buffer, sizeof(char)*256, 0);
    printf("%s\n",buffer);

    return 1;
 }

int Term_lcd(char** args) {

    if (args[1] == NULL) {
        perror( "lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
            return 1;
        }
        else {
            getcwd(directory,sizeof(directory));
            printf("%s\n", directory);
        }
    }

    return 1;
 }

int Term_lls(char** args) {

    DIR *mydir;
    struct dirent *myfile;

    mydir = opendir(directory);
    while((myfile = readdir(mydir)) != NULL)
    {

    printf(" %s\n", myfile->d_name);
    }
    closedir(mydir);

    return 1;
 }

int Term_lpwd(char** args) {

    printf("%s\n", directory);

    return 1;
 }

int Term_get(char** args) {

    if (args[1] == NULL) {
        printf("error: expected argument\n");
        return 1;
    }

    char *command = malloc(sizeof(char)*256);
    char *buffer = malloc(sizeof(char)*256);
    sprintf(command, "%s %s\0", args[0], args[1]);

    send(serverSocket, command, sizeof(char)*256, 0);
    recv(serverSocket, buffer, sizeof(char)*256, 0);

    if(strcmp(buffer, "No") == 0) {
        printf("File doesnt exist\n");
        return 1;
    }
    if(strcmp(buffer, "Stat") == 0) {
        printf("Cant read file stats\n");
        return 1;
    }

    char* sizeF = malloc(sizeof(char)*256);
    long int size;
    recv(serverSocket,sizeF,sizeof(char)*256,0);
    sscanf(sizeF,"%ld", &size);

    FILE* file;
    file = fopen(args[1], "wb");
    int read = 0;
    do {
        char buff[2048];
        int readed = recv(serverSocket,buff,sizeof(buff),0);
        fwrite(buff,1,readed,file);
        read += readed;
    } while (read < size);

    fclose(file);
    printf("Success\n");
    return 1;
 }

int Term_put(char** args) {

    if (args[1] == NULL) {
        printf("error: expected argument\n");
        return 1;
    }

    FILE* file;
    struct stat fileStat;

    if((file = fopen(args[1], "rb")) == NULL)
    {
        printf("File doesnt exist\n");
        return 1;
    }
    if (fstat(fileno(file), &fileStat) < 0)
    {
        printf("Cant read file stats\n");
        return 1;
    }


    char *command = malloc(sizeof(char)*256);
    sprintf(command, "%s %s\0", args[0], args[1]);
    send(serverSocket, command, sizeof(char)*256, 0);

    char* size = malloc(sizeof(char)*256);
    int int_size = 0;

    sprintf(size, "%ld", fileStat.st_size);
    int_size = fileStat.st_size;
    send(serverSocket, size, (sizeof(char)*256), 0);

    int ready = 0;
    while (ready < int_size) {
        char buff[2048];
        int k = fread(&buff, 1, 2048, file);
        ready += k;
        send(serverSocket, buff, k, 0);
    }

    fclose(file);
    printf("Success\n");
    return 1;
 }

int Term_exit(char** args) {

    send(serverSocket, args[0], sizeof(char)*256, 0);
    return 0;
 }



