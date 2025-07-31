/**
 * Chương trình này dùng để xóa những malloc đã được free trong file log
 * chỉ còn lại những malloc chưa được free
 * Sử dụng hashing (tốc độ O(1))
 */

/**
 * Một ý tưởn khác: DSLK, thêm tất cả các dòng có malloc vào DSLK (gồm địa chỉ và nội dung dòng đó), 
 * rồi khi gặp free thì tìm trong DSLK có malloc nào trùng với địa chỉ của free thì xóa node đó đi
 */

/**
 * So sánh DSLK và Hashing:
 * - DSLK: Tìm kiếm O(n), thêm O(1), xóa O(1)
 * - Hashing: Tìm kiếm O(1), thêm O(1), xóa O(1)
 * - Hashing khó đọc hơn, khó debug tường minh như DSLK. Hashing áp dụng được cho dữ liệu lớn hơn, hash_function cần tốt đó
 */
//pid malloc(1024)      = 0x12345678
//pid free(0x12345678)  = <void> 
#include "outp_log.h"

malloc_node_t *create_malloc_node(uint64_t addr, char *line)
{
    malloc_node_t *new_node = (malloc_node_t *)malloc(sizeof(malloc_node_t));
    if(new_node == NULL)
    {
        perror("malloc():");
        exit(EXIT_FAILURE);
    }

    /*address đang là mảng (trên heap) nên không thể gắn trực tiếp địa chỉ
    dùng con trỏ gán thẳng được địa chỉ (thế nhưng làm như vậy thì con trỏ address sẽ trỏ đến vùng stack và ko còn quản lý heap nữa)*/
    new_node->address = addr;;
    strcpy(new_node->line, line);
    new_node->next = NULL;

    return new_node;
}

void add_malloc_node(malloc_node_t **head, uint64_t addr, char *line)
{
    malloc_node_t *new_node = create_malloc_node(addr, line);

    //add the end of the list
    malloc_node_t *tmp = *head;
    if(*head == NULL)
    {
        (*head) = new_node;
    }

    while(tmp->next != NULL)
    {
        tmp = tmp->next;
    }

    tmp->next = new_node;
}

/*TH này dùng doubly linked list sẽ dễ hơn*/
void remove_malloc_node(malloc_node_t **head, uint64_t addr)
{
    malloc_node_t *tmp = *head;
    malloc_node_t *prev_tmp = NULL;
    if(tmp == NULL)
    {
        return;
    }

    while(tmp != NULL)
    {
        if(tmp->address == addr)
        {
            if(prev_tmp == NULL)  //the first node
            {
                *head = tmp->next;  //update the head
            }
            else
            {
                prev_tmp->next = tmp->next;  //update the next of previous node
            }
            free(tmp);  //free the memory of the removed node
            return;  
        }

        prev_tmp = tmp;
        tmp = tmp->next;
    }
}

/*How many free bytes are in heap?*/
void get_heap_free_bytes(pid_t pid, char *path_outfile)
{
    char path_logfile[BUFF_SIZE];
    sprintf(path_logfile, "./ltrace_%d.log", pid);
    FILE *ltrace_file = fopen(path_logfile, "r");
    if(ltrace_file == NULL)
    {
        perror("ltrace_file fopen()");
        exit(EXIT_FAILURE);
    }

    char line[BUFF_SIZE];
    malloc_node_t *head = NULL;
    while(fgets(line, BUFF_SIZE, ltrace_file) != NULL)
    {
        char *pos = NULL;
        if((pos = strstr(line, "malloc(")) != NULL)
        {
            char *addr_start = strchr(line, '=');   //addr_start is pointing to the '=' character, right after the '=' is the address
            if(addr_start != NULL)
            {
                uint64_t addr = 0;
                sscanf(addr_start + 1, "%lx", &addr);  //the string saved to &addr will be a hexadecimal number (%lx) -> sscanf
                add_malloc_node(&head, addr, line);
            }
        }
        else if((pos = strstr(line, "free(")) != NULL)
        {
            uint64_t addr = 0;
            if(sscanf(pos, "free(%lx)", &addr) == 1)
            {
                remove_malloc_node(&head, addr);
            }
        }
    }

    /*print the remaining malloc nodes*/
    FILE *output_file = fopen(path_outfile, "w");
    if(output_file == NULL)
    {
        perror("output_file fopen()");
        exit(EXIT_FAILURE);
    }

    malloc_node_t *tmp = head;
    while(tmp != NULL)
    {
        fputs(tmp->line, output_file);
        printf("%s", tmp->line);
        tmp = tmp->next;
    }
    fclose(ltrace_file);
}