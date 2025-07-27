#include <stdio.h>
#include <unistd.h>

int main (const int argc, const char *argv[])
{
    printf("Hello 123sam\n");
    printf("The process ID is %d\n", (int)getpid());
    printf("The parent process ID is %d\n", (int)getppid());    //là tk bash cỉa hệ thống
    while(1);       //kiểm tra PID đang chạy thôi
    return 0;
}