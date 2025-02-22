#include <stdio.h>
#include <stdlib.h>
#include "string.h"


void allocated_memory()
{
    char *ptr;
    ptr = (char *)malloc(sizeof(char ) * 10);
}

void main(int argc, char *argv[]) 
{   
    //memory leak
    allocated_memory();
    while(1);
    // int i;

    // // In ra số lượng command-line truyền vào.
    // printf("Number of arguments: %d\n", argc);
    // if(strcmp(argv[1], "1") == 0)
    // {
    //     printf("hello\n");
    // }    
    // else if(strcmp(argv[1], "2") ==0)
    // {
    //     printf("helloSam\n");
    // }
    // // In ra nội dung của mỗi command-line.
    // for (i = 0; i < argc; i++) {
    //     printf("argv[%d]: %s\n", i, argv[i]);
    // }
}