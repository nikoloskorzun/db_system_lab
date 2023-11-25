all:
	gcc process_pool/process_pool.c main_process_ref/main_process.c -o server.out -pthread