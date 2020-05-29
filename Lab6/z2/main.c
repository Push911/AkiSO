#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

int matrix1[1000][1000];
int matrix2[1000][1000];
int matrix3[1000][1000];
char matrixpom[1000];
pthread_mutex_t mutexes[1000];

typedef struct Thread_struct
{
    int id;
    int arg1;
    int arg2;
    int arg3;
    //int col;
    int row;
}Thread_struct;

void *multiply(void *arguments)
{
    Thread_struct* args;
    args = (Thread_struct*) arguments;
    int a=args->arg1;
    int b=args->arg2;
    int c=args->arg3;
    int row = 0;
    int col = 0;
    while(1)
    {
        if(matrixpom[row] == '\0')
        {
            if(pthread_mutex_trylock(&mutexes[row]) == 0)
            {
                if(matrixpom[row] == '\0')
                {
                    for(int k=0; k<c; k++)
                    {
                        for(int i=0; i<a; i++)
                        {
                            matrix3[row][k] ^= matrix1[row][i]*matrix2[i][k];
                        }

                    }
                    matrixpom[row] = '1';
                }
                pthread_mutex_unlock(&mutexes[row]);
            }
        }
        // col++;
        // if(col == args->arg3) {
        //     row++;
        //     col = 0;
        // }
        row++;
        if(row == args->arg2)
        {
            pthread_exit(NULL);
        }
    }
}

int main(int argc, char **argv)
{
    if(argc !=5)
    {
        fprintf(stderr, "usage: race threadsNum stringsize iterations\n");
        exit(1);
    }
    srand(time(NULL));
    int a=atoi(argv[1]);
    int b=atoi(argv[2]);
    int c=atoi(argv[3]);
    int i,j,k=0;
    int threadsNum = atoi(argv[4]);

    pthread_t* tid;
    void *retval;
    Thread_struct *args;

    tid = (pthread_t *) malloc(sizeof(pthread_t) * threadsNum);
    args = (Thread_struct *) malloc(sizeof(Thread_struct) *threadsNum);

    //losuje macierze A i B
    for(i=0; i<b; i++){
        for(j=0; j<a; j++){
            matrix1[i][j]=rand()%2;
        }
    }
    for(i=0; i<a; i++){
        for(j=0; j<c; j++){
            matrix2[i][j]=rand()%2;
        }
    }
    //zeruje macierz C
    for(i=0; i<b; i++){
        for(j=0; j<c; j++){
            matrix3[i][j]=0;
            //matrixpom[i][j]='\0';
        }
        matrixpom[i]='\0';
    }
    //wypełnia macierz C
    for(i=0; i<b; i++)
    {
        // for(j=0; j<c; j++){
        //     pthread_mutex_init(&mutexes[i][j], NULL);
        // }
        pthread_mutex_init(&mutexes[i], NULL);
    }
    int row = 0;
    //int col = 0;
    for(i=0; i<threadsNum; i++)
    {
        args[i].row=row;
        //args[i].col=col;
        args[i].arg1=a;
        args[i].arg2=b;
        args[i].arg3=c;
        pthread_create(tid+i, NULL, multiply, args+i);

        row++;
        if(row == b)
        {
            break;
        }
    }
    for(i=0; i<threadsNum; i++)
    {
        pthread_join(tid[i], &retval);
    }
}