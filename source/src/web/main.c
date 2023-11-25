#include "./main_process_ref/main_process.h"
#include "./process_pool/process_pool.h"

int main() {
    ProcessPool p_pool = create_process_pool(3);
    serve_forever(&p_pool);
    for(int i = 0; i < 3; i++) {
        pthread_join(p_pool->ready_listeners[i]);
    }
}
