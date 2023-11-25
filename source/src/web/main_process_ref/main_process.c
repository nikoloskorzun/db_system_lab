#include "main_process.h"

typedef struct 
{
	size_t content_len;
	unsigned status_code;
	char* data;
} response;

typedef struct 
{
	unsigned content_len;
	void* data;
} request;

void log_info(char* msg) 
{
   FILE *log_file = fopen("out.log", "a+");
   if (log_file == NULL) {
       printf("Error opening file!\n");
       return;
   }
   fprintf(log_file, "%s\n", msg);
   fclose(log_file);
}


int* endpoint_logic(request* req, int* answer_len, char* answer) 
{
	int cur_len = 6;
    int* data = (int*) req->data;
    char *command = (char*) req->data;
    log_info("Command obtained");
    answer = "Hello";
    log_info("Endpoint logic completed");
    *answer_len = strlen(answer);
}

response do_work(request* req) 
{
	if(req->data != NULL) {
		int* result_len = malloc(sizeof(int));
		int* result = malloc(100);
		endpoint_logic(req, result_len, result);
		response resp = {sizeof(int)*(*result_len)+sizeof(unsigned)*2, 200, (void*)result};
		return resp;
	}
	response resp = {sizeof(unsigned)*2, 500, NULL};
	return resp;
}

request receive_data(int sock) 
{
	float* buf = malloc(sizeof(float)*3);
	int bytes_read;
	unsigned content_length;
	recv(sock, &content_length, sizeof(unsigned), 0);
	recv(sock, buf, content_length-sizeof(unsigned), 0);
	request received = {content_length, buf};
	return received;
}

void respond(int sock, response *resp, int len, int flags) 
{
	send(sock, &resp->content_len, sizeof(unsigned), 0);
	send(sock, &resp->status_code, sizeof(unsigned), 0);
	send(sock, (void*)resp->data, resp->content_len - sizeof(unsigned)*2, 0);
}


void request_handle(sock) 
{
    
    if(sock < 0){
        perror("accept");
        exit(3);
    }
	request received_request = receive_data(sock);
    log_info("New request received");
	response resp = do_work(&received_request);
    log_info("Work done");
	respond(sock, &resp, resp.content_len, 0);
	free(received_request.data);
    printf("DONE");
}


void accept_ready(int listener, ProcessPool* process_pool)
{
    int sock;
	while(1)
    {
        log_info("ACCEPTING");
        sock = accept(listener, NULL, NULL);
        int *args = (int*)(malloc(sizeof(sock)));
        args[0] = sock;
        log_info("New accept");
        apply_async(process_pool, request_handle, args);
        log_info("Job applied");
	}
	close(sock);
}

void serve_forever(ProcessPool* process_pool) 
{
	int listener;
    struct sockaddr_in addr;
    int bytes_read;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    
    if(listener < 0)
    {
        perror("s:socket");
        exit(1);
    }
	printf("s:Listening socket...");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("s:bind");
        exit(2);
    }
    listen(listener, 1);
	printf("\ns:BINDED 8080\n");
	accept_ready(listener, process_pool);
}
