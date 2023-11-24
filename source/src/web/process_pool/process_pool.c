#include "process_pool.h"



void* execute_in_process(pid_t work_pid, void* (*func)(void*), void *args){
	// wraps process target
	// IT IS COMPLETELY RIGHT YOU MORRON
	if (getpid() == work_pid){

		void* res = (*func)(args);
		return res;
	}
	return NULL;
}

typedef struct ProcessPipe 
{
    FILE *input;
    FILE *output;
    pid_t sub_process_pid;
} ProcessPipe;

typedef struct ProcessPool
{
    unsigned main_pid;
    unsigned process_count;
    ProcessPipe* pipes;
} ProcessPool;


void create_subprocess(ProcessPipe* pipe_holder) {
    void *dup_args = {pipe_holder->output, STDOUT_FILENO};
    pid_t child = fork();
    execute_in_process(child, dup2, dup_args);
}


ProcessPipe* create_process_pipe(ProcessPipe* pipes, unsigned idx) 
{
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        exit(-1);
    };
    ProcessPipe cur_pipe = {
        pipefd[0], pipefd[1], 0
    };
    create_subprocess(&cur_pipe);
    pipes[idx] = cur_pipe;
    return &cur_pipe;
}


ProcessPool create_process_pool(unsigned process_count) {
    pid_t main_pid = getpid();
    ProcessPipe* pipes = (ProcessPipe*) malloc(sizeof(ProcessPipe) * process_count);
    ProcessPool pool = {main_pid, process_count, pipes};
    for(int i = 0; i < process_count; i++) {
        void *args = {pipes, i};
        ProcessPipe new_process = *(
            (ProcessPipe*) execute_in_process(main_pid, create_process_pipe, args)
        ); // get comm primitive
        
    }
}
