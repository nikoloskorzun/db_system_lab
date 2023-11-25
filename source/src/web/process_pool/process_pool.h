#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <time.h>

void* execute_in_process(pid_t work_pid, void* (*func)(void*), void *args);
typedef struct ProcessPipe
{
    FILE *input;
    FILE *output;
    pid_t sub_process_pid;
};

typedef struct ProcessPool
{
    unsigned main_pid;
    unsigned process_count;  // 
    ProcessPipe* pipes;  // all pipes
    unsigned *idle_processes;  // [0, 1, 1, 0, 0] where index(0) - occupied process in pipes, 1 - free process
    pthread_t *ready_listeners;
};
ProcessPool create_process_pool(unsigned process_count);
void apply_async(ProcessPool* ppool, void* (*func)(void*), void *args)
