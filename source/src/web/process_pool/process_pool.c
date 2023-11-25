#include "process_pool.h"

void* execute_in_process(pid_t work_pid, void* (*func)(void*), void *args){
	// wraps process target
	if (getpid() == work_pid){
		void* res = (*func)(args);
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
    if (!work_info) {
        perror("malloc failed");
        return;
    }
    wait_for(3);

    while (1) {
        log_info1("Start scanning");
        if (fscanf(p_pipe->output, "%127s", work_info) != 1) {
            perror("fscanf failed or end of file reached");
            break;
        }
    }
    process_queue[idx] = 1;
    free(work_info); // Free the allocated memory
}

void wait_for (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}


void log_info1(char* msg) 
{
   FILE *log_file = fopen("out.log", "a+");
   if (log_file == NULL) {
       printf("Error opening file!\n");
       return;
   }
   fprintf(log_file, "%s\n", msg);
   fclose(log_file);
}


typedef struct {
    ProcessPool* ppool;
    void* (*func)(void*);
    void* args;
    int occupied_pos;
} AsyncArgs;


void apply_async(ProcessPool* ppool, void* (*func)(void*), void *args) {
    ProcessPipe* free_communication = NULL;
    log_info1("Entered apply_async");
    int occupied_pos = -1;
    for (int i = 0; i < ppool->process_count; i++) {
        if(ppool->idle_processes[i]) {
            free_communication = &ppool->pipes[i];
            occupied_pos = i;
            break; // break to exit the loop once a free communication is found
        }
    }
    
    if (!free_communication) {
        wait_for(0.05);
        return apply_async(ppool, func, args);
    }
    log_info1("Idle process found");
    pthread_t thread_id;
    ppool->idle_processes[occupied_pos] = 0;
    int rc = pthread_create(&thread_id, NULL, listen_to_process_ready, ppool->idle_processes, occupied_pos);
    log_info("New thread started");    
    log_info1("Idle filled");

    execute_in_process(free_communication->sub_process_pid, func, args);
    log_info1("Executed task");
    ppool->ready_listeners[occupied_pos] = thread_id;
    log_info1("Listener occupied");
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
        idle_processes[i] = 1;
        PipeArgs* args = malloc(sizeof(PipeArgs));  // Allocate memory for the arguments
        args->pipes = pipes;
        args->idx = i;
        execute_in_process(main_pid, create_process_pipe, args);
        free(args);  // Free the allocated memory
    }
    return pool;
}
