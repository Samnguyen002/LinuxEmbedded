#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])   /* Cấp phát stack frame cho hàm main() */
{
    /* code */
    pid_t child_pid;                /* Lưu trong stack frame của main() */
    int status, retrunValue;

    child_pid = fork();         
    if (0 == child_pid) {       /* Process con */
        printf("Im the child process, my PID: %d\n", getpid());
        printf("Child process terminate after 5 seconds\n");
	    sleep(5);
	    exit(0);

    } else if(child_pid > 0) {                     /* Process cha */
	    //while(1);
        retrunValue = wait(&status);    /*hàm wait() trả về giá trị pid của tk con*/
        if (retrunValue == -1) {
        printf("wait() unsuccessful\n");
        }

        printf("\nIm the parent process, PID child process: %d\n", retrunValue);
            
           // if (WIFEXITED(status)) {
             //   printf("Normally termination, status = %d\n", WEXITSTATUS(status));
           // } else if (WIFSIGNALED(status)) {
            //    printf("killed by signal, value = %d\n", WTERMSIG(status));
            //} 
    }

    return 0;
}