# !/bin/bash
gcc web/main.c web/process_pool/process_pool.c web/main_process_ref/main_process.c -o server.out -pthread -g -w
gcc -w -g client/client.c -o client.out
./server.out &
sleep 1
./client.out &
sleep 2
pkill server.out
