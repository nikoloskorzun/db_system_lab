#include "./process_pool/process_pool.h"
#include "./main_process_ref/main_process.h"

#include <pthread.h>

int main() {
    ProcessPool p_pool = create_process_pool(3);
    void* args = {&p_pool};
    
    execute_in_process(p_pool.main_pid, serve_forever, args);
    
    for(int i = 0; i < 3; i++) {
        void* args_join = {p_pool.ready_listeners[i], NULL};
        execute_in_process(p_pool.main_pid, pthread_join, args_join);
    }
    return 0;
}
