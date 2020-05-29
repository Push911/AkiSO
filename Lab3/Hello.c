#include <stdio.h>

#define KNRM  "\x1B[0m"
#define KBLC  "\x1B[30m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[91m"
#define BKRD  "\x1B[41m"

int main()
{
    printf("%sHello, World!\n", KRED);
    printf("%sHello, World!\n", KBLC);
    printf("%sHello, World!\n", KGRN);
    printf("%sHello, World!\n", KYEL);
    printf("%sHello, World!\n", KBLU);
    printf("%sHello, World!\n", KMAG);
    printf("%sHello, World!\n", KCYN);
    printf("%sHello, World!\n", KWHT);
    printf("%sHello, World!\n", KNRM);
    printf("%sbackgroundred\n", BKRD);
    return 0;
}
