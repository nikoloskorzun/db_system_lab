#ifndef __MAIN__PROCESS__H
#define __MAIN__PROCESS__H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <malloc.h>
#include <math.h>
#include "../process_pool/process_pool.h"
#include <pthread.h>
#include <string.h>

void serve_forever(ProcessPool* process_pool);
#endif