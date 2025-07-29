#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>  //open(), create()
#include <sys/wait.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <pthread.h>

#define FILE_PATH "./file.txt"
#define BUFFER_SIZE 256
#define PROCESS_NUMBER 5   
#define handle_err(msg){ perror(msg); exit(EXIT_FAILURE);}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;   //allocted Mutex lock

typedef struct {
    pid_t pid;
    char *process_name;  
} process_info_t;

pid_t get_pid(FILE **fp, char *process_name)     // Get the PID of a process by its name
{
    char command[50];
    if(sprintf(command, "pidof %s", process_name) < 0)     //return the number of charactes
    {
        handle_err("sprintf()");
    }

    *fp = popen(command, "r");
    if(fp == NULL)
    {
        handle_err("popen()");
    }

    char buff[256];
    if(fgets(buff, 256, *fp) == NULL)
    {
        handle_err("fgets()");
    }
    //printf("Pid of this process: %d\n", atoi(buff));
    return atoi(buff);
}

uint32_t get_total_mem()
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if(fp == NULL)
    {
        handle_err("get_total_mem fopen()");
    }

    char buff_meminfo[BUFFER_SIZE];
    uint32_t total_mem = 0;

    while(fgets(buff_meminfo, BUFFER_SIZE, fp) != NULL)
    {
        if(strncmp(buff_meminfo, "MemTotal:", 9) == 0)
        {
            sscanf(buff_meminfo + 9, "%d", &total_mem);
            break;
        }
    }

    fclose(fp);
    return total_mem;
}

uint32_t get_vsz_rss(pid_t pid)
{
    /*read the smaps file of the processes -> caculate the capacity of the virtual memory
    and the capacity of RAM used (RSS)*/
    char smaps_file[BUFFER_SIZE];
    sprintf(smaps_file, "/proc/%d/smaps", pid);
    FILE *fp = fopen(smaps_file, "r");
    if(fp == NULL)
    {        
        handle_err("get_vsz_rss fopen()");
    }

    char buff_smaps[BUFFER_SIZE];
    uint32_t total_size = 0, total_rss = 0;
    while(fgets(buff_smaps, BUFFER_SIZE, fp) != NULL)
    {
        if(strncmp(buff_smaps, "Size:", 5) == 0)
        {
            uint32_t size;
            sscanf(buff_smaps + 5, "%d", &size);
            total_size += size;
        }
        else if(strncmp(buff_smaps, "Rss:", 4) == 0)
        {
            uint32_t rss;
            sscanf(buff_smaps + 4, "%d", &rss);
            total_rss += rss;
        }
    }
    
    printf("Total size of virtual memory: %d KB\n", total_size);
    printf("Total VmRSS (Resident Set Size): %d KB\n", total_rss);

    fclose(fp);
    return total_rss;  
}

typedef struct 
{
    uint32_t VmSize;
    uint32_t VmRSS;
}mem_info_t;

mem_info_t get_stack(pid_t pid)
{
    /*read the smaps file of the processes -> caculate the capacity of the stack memory*/
    char smaps_file[BUFFER_SIZE];
    sprintf(smaps_file, "/proc/%d/smaps", pid);
    FILE *fp = fopen(smaps_file, "r");
    if(fp == NULL)
    {
        handle_err("get_rss_stack fopen()");
    }

    char buff_stacksmaps[BUFFER_SIZE];
    mem_info_t stack_mem;
    bool stack_point = false;

    while(fgets(buff_stacksmaps, BUFFER_SIZE, fp) != NULL)
    {
        if(strstr(buff_stacksmaps, "[stack]") != NULL)
        {
            //This is stack memory
            stack_point = true;
            continue;  // continue to next line, this is the stack information
        }

        if(stack_point)
        {
            if(strncmp(buff_stacksmaps, "Size:", 5) == 0)
            {
                sscanf(buff_stacksmaps + 5, "%d", &stack_mem.VmSize);
            }
            else if(strncmp(buff_stacksmaps, "Rss:", 4) == 0)
            {
                sscanf(buff_stacksmaps + 4, "%d", &stack_mem.VmRSS);
                break; //done
            }
        }
    }

    fclose(fp);
    return stack_mem;  
}

mem_info_t get_heap(pid_t pid)
{
    /*read the smaps file of the processes -> caculate the capacity of the stack memory*/
    char smaps_file[BUFFER_SIZE];
    sprintf(smaps_file, "/proc/%d/smaps", pid);
    FILE *fp = fopen(smaps_file, "r");
    if(fp == NULL)
    {
        handle_err("get_rss_stack fopen()");
    }

    char buff_heapsmaps[BUFFER_SIZE];
    mem_info_t heaps_mem;
    bool heap_point = false;

    while(fgets(buff_heapsmaps, BUFFER_SIZE, fp) != NULL)
    {
        if(strstr(buff_heapsmaps, "[stack]") != NULL)
        {
            //This is heap memory
            heap_point = true;
            continue;  // continue to next line, this is the heap information
        }

        if(heap_point)
        {
            if(strncmp(buff_heapsmaps, "Size:", 5) == 0)
            {
                sscanf(buff_heapsmaps + 5, "%d", &heaps_mem.VmSize);
            }
            else if(strncmp(buff_heapsmaps, "Rss:", 4) == 0)
            {
                sscanf(buff_heapsmaps + 4, "%d", &heaps_mem.VmRSS);
                break; //done
            }
        }
    }

    fclose(fp);
    return heaps_mem;  
}

char *get_libc_path(pid_t pid)
{
    /*get the path of libc.so.6*/
    char buff_maps[BUFFER_SIZE] = {0};

    sprintf(buff_maps, "/proc/%d/maps", pid);
    FILE *maps_file = fopen(buff_maps, "r");
    if(maps_file == NULL)
    {
        handle_err("maps_file fopen()");
    }

    char line[BUFFER_SIZE];
    while(fgets(line, BUFFER_SIZE, maps_file) != NULL)
    {
        if(strstr(line, "libc.so.6") != NULL)
        {
            char *path_start = strchr(line, '/');  // return a pointer to the first of '/'
            //but path_start is pointing to the "line" buffer, it is a ring buffer, discard when function terminate
            if(path_start != NULL)
            {
                char *libc_path = strdup(path_start);  
                char *end = strchr(libc_path, '\n'); 
                if(end != NULL)
                {
                    *end = '\0'; 
                    fclose(maps_file);
                    return libc_path;
                }
            }
        }
    }
    
    fclose(maps_file);
    return NULL;
}

void count_malloc_free(pid_t pid)
{
    /*count the number of malloc and free calls using perf*/
    char buff_perf[BUFFER_SIZE];
    sprintf(buff_perf, "sudo perf stat -e probe_libc:malloc -e probe_libc:free -p %d sleep 10", pid);
    FILE *perf = popen(buff_perf, "r");
    if (perf == NULL) 
    {
        handle_err("popen() for perf");
    }

    char line[BUFFER_SIZE];
    int malloc_count = 0, free_count = 0;
    while (fgets(line, BUFFER_SIZE, perf)) 
    {
        if (strstr(line, "probe_libc:malloc")) 
        {
            sscanf(line - 7, "%d", &malloc_count);
            printf("Number of malloc: %d\n", malloc_count);
        }
        else if (strstr(line, "probe_libc:free")) 
        {
            sscanf(line - 7, "%d", &free_count);
            printf("Number of free: %d\n", free_count);
        }
    }
    pclose(perf);
}

void dump_mem_to_screen(pid_t pid)
{
    uint64_t start_address = 0, end_address = 0;
    uint32_t inode = 0;
    char perm[4], offset[10], device[6];
    char pathname[BUFFER_SIZE];   // Allocate memory for pathname, avoid using restricted memory
    
    char buff_maps[BUFFER_SIZE] = {0};

    sprintf(buff_maps, "/proc/%d/maps", pid);
    FILE *maps_file = fopen(buff_maps, "r");
    if(maps_file == NULL)
    {
        handle_err("maps_file fopen()");
    }

    char line[BUFFER_SIZE];
    while(fgets(line, BUFFER_SIZE, maps_file) != NULL)
    {
        //lx printf the unsigned long to hexadecimal format
        sscanf(line, "%lx-%lx %s %s %s %d %s", &start_address, &end_address, perm, offset, device, &inode, pathname);
        printf("0x%lx - 0x%lx %s %s %s %d\t%s\n", start_address, end_address, perm, offset, device, inode, pathname);
    }
}

void *monitor_process(void *args)
{
    process_info_t *process_info = (process_info_t *)args;
    uint32_t total_mem = get_total_mem();

    //set up perf probe for malloc and free
    // char *libc_path = get_libc_path(process_info->pid);
    // printf("Libc path: %s\n", libc_path);
    // char buff_perf[BUFFER_SIZE];
    // sprintf(buff_perf, "sudo perf probe -x %s malloc", libc_path);
    // system(buff_perf);
    // sprintf(buff_perf, "sudo perf probe -x %s free", libc_path);
    // system(buff_perf);

    for(;;)
    {
        pthread_mutex_lock(&lock);
        uint32_t rss = get_vsz_rss(process_info->pid);
        mem_info_t stack_mem = get_stack(process_info->pid);
        mem_info_t heap_mem = get_heap(process_info->pid);
        printf("-----------------------------PID %d--------------------------------\n", process_info->pid);
        printf("Name: %s\n",process_info->process_name);
        printf("Total memory of this system (RAM): %d KB\n", total_mem);
        printf("\t_____\tVmSIZE\tVmRSS\n");
        printf("\tTotal\t      \t%d\tKB\n", rss);
        printf("\tStack\t%d\t%d\tKB\n", stack_mem.VmSize, stack_mem.VmRSS);
        printf("\tHeap\t%d\t%d\tKB\n", heap_mem.VmSize, heap_mem.VmRSS);
        printf("------------------------------------------------------------------------\n");
        dump_mem_to_screen(process_info->pid);
        printf("________________________________________________________________________\n");
        printf("________________________________________________________________________\n");
        pthread_mutex_unlock(&lock);
        count_malloc_free(process_info->pid);    //sleep(10) is inside this function
        //sleep(10);
    }

    //free(libc_path);  // strdup la su dung malloc, can phai free
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage: ./<exec_file> <file.txt>\n");
        exit(EXIT_FAILURE);
    }

    /*1, Lay tung pathname (process_name) cua process*/
    FILE *fp;
    int fd;     //int 2 4 6 8, fd = any value (big endian, little endian)
    char buff_file[BUFFER_SIZE];
    process_info_t process_info[PROCESS_NUMBER];
    pthread_t thread_id[PROCESS_NUMBER];

    fd = open(FILE_PATH, O_RDONLY);
    if(fd == -1)
    {
        handle_err("open()");
    }

    int len = read(fd, buff_file, BUFFER_SIZE);
    if(len  == -1)
    {
        handle_err("read()");
    }

    int start = 0;                      //tach cac pathname
    int index_name = 0; 
    for(int i = 0; i < len; i++)
    {
        if(buff_file[i] == '\n')
        {
            buff_file[i] = '\0';
            process_info[index_name].process_name = &buff_file[start];    //phan tu file_name[] sẽ trỏ tới từng đầu chuỗi, sau '\0'
            start = i + 1;
            printf("Process name %d: %s\n", index_name, process_info[index_name].process_name);    //truongf hợp này vân phải 'enter' o chuoi cuoi
            index_name++;
        }
        //printf("%c\n", buff_file[i]);
    }
    wait(NULL);

    /*2, Lay pid tu pathname*/
    // int pid_process = (int)get_pid(&fp, process_info[1].process_name);

    // //for i -> index_name - 1 --> how many process names
    // printf("Pid of this process: %d\n", get_pid(&fp, process_info[0].process_name));
    // printf("Pid of this process: %d\n", get_pid(&fp, process_info[1].process_name));
    // printf("The total memory (RAM) of this system: %d KB\n", get_total_mem());

    int process_number = index_name;  //update the number of processes
    for(int i = 0; i < process_number; i++)
    {
        process_info[i].pid = get_pid(&fp, process_info[i].process_name);
        printf("Process name: %s, PID: %d\n", process_info[i].process_name, process_info[i].pid);

        char buff_perf[BUFFER_SIZE];
        char *libc_path = get_libc_path(process_info[i].pid);
        sprintf(buff_perf, "sudo perf probe -x %s malloc", libc_path);
        system(buff_perf);
        sprintf(buff_perf, "sudo perf probe -x %s free", libc_path);
        system(buff_perf);

        free(libc_path);  // free the libc_path after using it

        if(pthread_create(&thread_id[i], NULL, monitor_process, (void *)&process_info[i]))
        {
            printf("Error thread %i\n", i);
            handle_err("pthread_create()");
        }
    }

    for(int i = 0; i < process_number; i++)
    {
        if(pthread_join(thread_id[i], NULL))
        {
            printf("Error joining thread %i\n", i);
            handle_err("pthread_join()");
        }
    }

    /*3, Doc kich thuoc cua process,
    // dung statm files*/ 
    // char buff_statm[BUFFER_SIZE] = {0};
    // char line[BUFFER_SIZE];
    // uint64_t VmSize_KB = 0;

    // sprintf(buff_statm, "/proc/%d/statm", pid_process);
    // FILE *statm_files = fopen(buff_statm, "r");
    // if(statm_files == NULL)
    // {
    //     handle_err("statm_files fopen()");
    // }

    // while(fgets(line, BUFFER_SIZE, statm_files) != NULL)
    // {
    //     char *VmSize = strtok(line, " ");
    //     VmSize_KB = 4*strtol(VmSize, NULL, 10);     //may change strol to atoi()
    //     printf("the VM capacity of this proccess (%d): %ld KB\n", pid_process, VmSize_KB);
    // }


    pclose(fp);
    return 0;
}