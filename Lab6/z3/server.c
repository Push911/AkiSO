#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>

void Cycle();
void Login(int sock);
char* ReadLine (int sock);
char** SplitLine(char* line);
void Execute (char** args, int sock);

int Serv_cd(char** args, int sock);
int Serv_ls(char** args, int sock);
int Serv_pwd(char** args, int sock);
int Serv_get(char** args, int sock);
int Serv_put(char** args, int sock);
int Serv_exit(char** args, int sock);

char *builtin_str[] = {
        "cd",
        "ls",
        "pwd",
        "get",
        "put",
        "exit" };
int (*builtin_func[]) (char **, int) = {
        &Serv_cd,
        &Serv_ls,
        &Serv_pwd,
        &Serv_get,
        &Serv_put,
        &Serv_exit };

int serverFd;
struct sockaddr_in serverAddress, clientAddress;
fd_set activeFd, readFd;

char * users[] = {"root", "user1", "user2", "user3", "user4"};
char * passwords[] = {"1", "qwerty1", "qwerty2", "qwerty3", "qwerty4"};
int users_amount = 5;
char* const errorDis = "ERROR_CLIENT_DISABLED\0";

int main ()
{

    //// socket() //
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverFd == -1) {
        perror("socket error : ");
        exit (1);
    }
    ////


    //// sockaddr_in //
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family       = AF_INET;
    serverAddress.sin_addr.s_addr  = htonl(INADDR_ANY);
    serverAddress.sin_port         = htons(7621);
    ////

    //// bind() //
    if(bind(serverFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) > 0)
    {
        perror("bind error : ");
        exit(0);
    }
    ////

    //// listen() //
    if(listen(serverFd, 5) != 0) {
        perror("listen error : ");
    }
    ////

    //// select //
    FD_ZERO (&activeFd);
    FD_SET (serverFd, &activeFd);
    ////

    Cycle();

    return 0;
}

void Cycle ()
{
    int newSocket, address;

    while (1)
        {
        readFd = activeFd;
        if (select (FD_SETSIZE, &readFd, NULL, NULL, NULL) < 0)
        {
            perror ("select");
            exit (EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET (i, &readFd))
            {
                if (i == serverFd)
                {
                    address = sizeof (clientAddress);
                    newSocket = accept(serverFd,(struct sockaddr *)&clientAddress,&address);
                    if (newSocket < 0)
                    {
                        perror ("accept");
                        exit (1);
                    }
                    fprintf (stderr,"Server: connect from host %s, port %hd.\n",inet_ntoa (clientAddress.sin_addr),ntohs (clientAddress.sin_port));
                    FD_SET (newSocket, &activeFd);
                    Login(newSocket);
                }
                else
                {

                    Execute(SplitLine(ReadLine(i)),i);
                }
            }

    }
}

char* ReadLine (int sock) {

    int bytes_read;
    char *line = malloc(sizeof(char)*256);

    bytes_read = recv(sock, line, sizeof(char)*256, 0);
    if (bytes_read <= 0) {

        close (sock);
        FD_CLR(sock, &activeFd);
        printf("\nclient disabled\n\tsocket: %d\n", sock);
        line = errorDis;
    }
    return line;
}

char** SplitLine(char* line) {

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

void Execute (char** args, int sock) {

    if (args[0] == errorDis)
        return;

    for (int i = 0; i < 6; i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            (*builtin_func[i])(args, sock);
        }
    }
}

void Login (int sock) {

    char* user_temp = malloc(256 * sizeof(char));
    char* passwd_temp = malloc(256 * sizeof(char));
    char** tmpArray;

    while(1) {
        user_temp  = ReadLine(sock);
        if (user_temp == errorDis)
            return;

        passwd_temp  = ReadLine(sock);
        if (passwd_temp == errorDis)
            return;

        tmpArray = SplitLine(user_temp);
        user_temp = tmpArray[0];

        tmpArray = SplitLine(passwd_temp);
        passwd_temp = tmpArray[0];


        printf("\nuser is trying to connect:\n\tsocket: %d\n\tlogin: %s\n\tpassword: %s\n", sock, user_temp, passwd_temp);

        for(int i = 0; i < users_amount; i++) {
            if(strcmp(users[i], user_temp) == 0 && strcmp(passwords[i], passwd_temp) == 0)
            {
                printf("success\n");
                send(sock, "0", sizeof("0"), 0);
                return;
            }
        }
        printf("error\n");
        send(sock, "1", sizeof("1"), 0);
    }
}

int Serv_cd(char** args, int sock) {

    printf("\n\tuser socket: %d\n\tcommand: %s %s\n", sock, args[0], args[1]);
    if (chdir(args[1]) != 0) {
        perror("lsh");
        send(sock, "No such file or directory",sizeof(char)*256 , 0);
    }
    char directory[PATH_MAX];
    getcwd(directory,sizeof(directory));
    send(sock, directory,sizeof(char)*256 , 0);
}

int Serv_ls(char** args, int sock) {

    printf("\n\tuser socket: %d\n\tcommand: %s\n", sock, args[0]);
    DIR *mydir;
    struct dirent *myfile;
    char directory[PATH_MAX];
    getcwd(directory,sizeof(directory));
    char*  buffer = malloc( strlen("1"));

    mydir = opendir(directory);
    while((myfile = readdir(mydir)) != NULL)
    {
        send(sock, "1", strlen("1"), 0);
        recv(sock, buffer, strlen("1"), 0);
        send(sock, myfile->d_name,sizeof(char)*256, 0);
        recv(sock, buffer, strlen("1"), 0);
    }
    send(sock, "0", strlen("1"), 0);
    recv(sock, buffer, strlen("1"), 0);
    closedir(mydir);
    return 0;
}

int Serv_pwd(char** args, int sock) {

    printf("\n\tuser socket: %d\n\tcommand: %s\n", sock, args[0]);
    char directory[PATH_MAX];
    getcwd(directory,sizeof(directory));
    send(sock, directory,sizeof(char)*256, 0);
    return 0;
}

int Serv_get(char** args, int sock) {

    printf("\n\tuser socket: %d\n\tcommand: %s %s\n", sock, args[0], args[1]);

    struct stat fileStat;
    char* statusFile = malloc(sizeof(char)*256);
    sprintf(statusFile,"normal");
    FILE* file;
    char* size = malloc(sizeof(char)*256);
    int int_size = 0;

    if((file = fopen(args[1], "rb")) == NULL)
    {
        sprintf(statusFile,"No");
        return 0;
    }
    if (fstat(fileno(file), &fileStat) < 0)
    {
        sprintf(statusFile,"Stat");
        return 0;
    }
    send(sock, statusFile, sizeof(statusFile), 0);

    sprintf(size, "%ld", fileStat.st_size);
    int_size = fileStat.st_size;
    send(sock, size, (sizeof(char)*256), 0);

    int ready = 0;
    while (ready < int_size) {
        char buff[2048];
        int k = fread(&buff, 1, 2048, file);
        ready += k;
        send(sock, buff, k, 0);
    }

    fclose(file);
    return 0;
}

int Serv_put(char** args, int sock) {

    printf("\n\tuser socket: %d\n\tcommand: %s %s\n", sock, args[0], args[1]);

    char* sizeF = malloc(sizeof(char)*256);
    long int size;
    recv(sock,sizeF,sizeof(char)*256,0);
    sscanf(sizeF,"%ld", &size);

    FILE* file;
    file = fopen(args[1], "wb");
    int read = 0;
    do {
        char buff[2048];
        int readed = recv(sock,buff,sizeof(buff),0);
        fwrite(buff,1,readed,file);
        read += readed;
    } while (read < size);

    fclose(file);
    return 1;
}

int Serv_exit(char** args, int sock) {

    printf("\n\tuser socket: %d\n\tcommand: %s\n", sock, args[0]);
    close (sock);
    FD_CLR(sock, &activeFd);
    printf("\tclient disabled\n\tsocket: %d\n", sock);

    return 0;
}