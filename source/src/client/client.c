#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


struct request {
	char *data;
};

struct response {
	unsigned content_len;
	unsigned status_code;
	char *data;
};

int main()
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
       	perror("connect");
        exit(2);
	}
	printf("Connected \n");
	char* data = "Hello world";
	struct request on_send = { data };
    send(sock, on_send.data, 12, 0);
    printf("Sended data \n");
	struct response resp;
	unsigned cl;
	recv(sock, &cl, 4, 0);
	printf("RECEIVED CONTENT LEN: %u\n", cl);
	resp.content_len = cl;
	printf("RECEIVED CONTENT\n");
	recv(sock, &resp.status_code, sizeof(unsigned), 0);
	char* resp_data;
	if(resp.status_code == 200) {
		recv(sock, &resp_data, cl-2*sizeof(unsigned), 0);
        printf("ANSWER %i\n", resp_data);
	} else {
		printf("Status: %i", resp.status_code);
	}
    close(sock);

	return 0;
}