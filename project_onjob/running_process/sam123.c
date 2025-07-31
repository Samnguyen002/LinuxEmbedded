#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <valgrind/memcheck.h>

int main (const int argc, const char *argv[])
{
    printf("Hello sam123\n");
    printf("The process ID is %d\n", (int)getpid());
    printf("The parent process ID is %d\n", (int)getppid());  

    int count = 0;
    while(1)
    {
        char *ptr = (char *)malloc(1024);
        if (!ptr) 
        {
            perror("malloc failed");
            break;
        }

        memset(ptr, 0xAB, 1024);
        printf("Da cap phat %d KB vao bo nho ao\n", ++count);

        if(count %2 == 0)
        {
            free(ptr);
        }
        //VALGRIND_DO_LEAK_CHECK;
        // for(int i = 0; i < 100000000; i++);
        // for(int i = 0; i < 100000000; i++);
        // for(int i = 0; i < 100000000; i++);
        // for(int i = 0; i < 100000000; i++);
        sleep(5);
    }      
    return 0;
}
