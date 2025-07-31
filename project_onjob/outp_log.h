#ifndef OUTP_LOG_H
#define OUTP_LOG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "uthash.h"

//Use a singly linked list to store the lines storing/containing the "malloc"

#define BUFF_SIZE 256

typedef struct malloc_node
{
    uint64_t address;           //the address of malloc
    char line[BUFF_SIZE];       //the line storing the malloc
    struct malloc_node *next;
}malloc_node_t;

malloc_node_t *create_malloc_node(uint64_t , char *);
void add_malloc_node(malloc_node_t **, uint64_t , char *);
void remove_malloc_node(malloc_node_t **, uint64_t );
void get_heap_free_bytes(pid_t , char *);

#endif //OUTP_LOG_H