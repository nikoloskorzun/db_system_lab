#include "process_pool.h"

void* execute_in_process(pid_t work_pid, void* (*func)(void*), void *args){
	// wraps process target
	if (getpid() == work_pid){
		void* res = (*func)((int*)args);
        
		return res;
	}
	return NULL;
}

void create_subprocess(ProcessPipe* pipe_holder) {
    void *dup_args = {pipe_holder->output, STDOUT_FILENO};
    pid_t child = fork();
    execute_in_process(child, dup2, dup_args);
}


void listen_to_process_ready(ProcessPipe* p_pipe, unsigned* process_queue, int idx) {
    char* work_info = (char*) malloc(sizeof(char) * 128);
    while(work_info != "DONE") {
        int content_length;
        read(p_pipe->output, &content_length, sizeof(unsigned));
        read(p_pipe->output, work_info, 1);
        printf("[%u] - %s", p_pipe->sub_process_pid, work_info);
    }
    process_queue[idx] = 1;
}

void wait_for (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

void apply_async(ProcessPool* ppool, void* (*func)(void*), void *args) {
    ProcessPipe* free_communication = NULL;
    int occupied_pos = -1;
    for (int i = 0; i < ppool->process_count; i++) {
        if(ppool->idle_processes[i]) {
            free_communication = &ppool->pipes[i];
            occupied_pos = i;
            break; // Added break to exit the loop once a free communication is found
        }
    }

    if (!free_communication) {
        wait_for(0.05);
        return apply_async(ppool, func, args);
    }

    pthread_t thread_id;
    int rc = pthread_create(&thread_id, NULL, listen_to_process_ready, free_communication);
    if (rc) {
        // Handle pthread_create error
        return;
    }
    ppool->idle_processes[occupied_pos] = 0;
    execute_in_process(free_communication->sub_process_pid, func, args);
    ppool->ready_listeners[occupied_pos] = thread_id;
}


typedef struct {
    ProcessPipe* pipes;
    int idx;
} PipeArgs;


ProcessPipe* create_process_pipe(PipeArgs* args) {
    PipeArgs* arguments = (PipeArgs*)args;
    ProcessPipe* pipes = arguments->pipes;
    int idx = arguments->idx;
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        exit(-1);
    };
    
    ProcessPipe cur_pipe = {
        pipefd[0], pipefd[1], 0
    };
    if (idx < 0 || idx >= 3) {
        fprintf(stderr, "Index out of bounds\n");
        return NULL;
    }
    pipes[idx] = cur_pipe;
    create_subprocess(&cur_pipe);    
    return &cur_pipe;
}


ProcessPool create_process_pool(unsigned process_count) {
    pid_t main_pid = getpid();
    ProcessPipe* pipes = (ProcessPipe*) malloc(sizeof(ProcessPipe) * process_count);

    if (!pipes) {
        fprintf(stderr, "Failed to allocate memory for pipes\n");
        exit(-1);
    }

    unsigned *idle_processes =  (unsigned*) malloc(sizeof(unsigned) * process_count);
    pthread_t *ready_listeners = (pthread_t*) malloc(sizeof(pthread_t) * process_count);
    ProcessPool pool = {main_pid, process_count, pipes, idle_processes, ready_listeners};
    for(int i = 0; i < process_count; i++) {
        PipeArgs* args = malloc(sizeof(PipeArgs));  // Allocate memory for the arguments
        args->pipes = pipes;
        args->idx = i;
        execute_in_process(main_pid, create_process_pipe, args);
        free(args);  // Free the allocated memory
    }
    return pool;
}
