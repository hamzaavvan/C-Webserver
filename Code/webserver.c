#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <pthread.h>

#define req_len 2048

#define HEADER_SZ 500
#define STATUS_SZ 20

struct ext
{
	char name[100];
	char value[100];
};
struct process {
		char *request;
		int  fd_client;
		char *path;
};

// List of server's supported extensions 
struct ext extensions[] = {
	{"html", "text/html; charset=UTF-8"}, {"css", "text/css"},
 	{"js", "application/js"}, {"jpg", "image/jpeg"},
 	{"png", "image/png"},  
};

int ext_len = sizeof(extensions)/sizeof(extensions[0]);
char resp_headers[] = "HTTP/1.1";


void prepare_status(FILE *fd, char *status) {
	if (fd == NULL) {
		strcpy(status, " 404 Not Found\r\n");
	} else {
		strcpy(status, "  200 OK\r\n");
	}
}

void prepare_content_type(FILE *fd, char file[], char *content_type) {
	char *token = strtok(file, ".");
	char ext[10];

	while(token != NULL) {
		strcpy(ext, token);
		token = strtok(NULL, ".");
	}

	for (int i = 0; i < ext_len; ++i)
	{
		if (!strcmp(ext, extensions[i].name)  || fd == NULL) {
			strcat(content_type, extensions[i].value);
			break;
		}
	}

	strcat(content_type, "\r\n");
}

void prepare_headers(FILE *fd, char file[], int length) {
	char sign[] = "Server: C/1.0 \r\n";
	char split[] = "\r\n";
	
	char content_length[50];
	sprintf(content_length, "%s%d%s", "Content-Length: ", length, "\r\n");

	char status[STATUS_SZ];
	prepare_status(fd, status);

	char content_type[100] = "Content-type: ";
	prepare_content_type(fd, file, content_type);

	strcat(resp_headers, status);
	strcat(resp_headers, sign);
	strcat(resp_headers, content_type);
	strcat(resp_headers, content_length);
	strcat(resp_headers, split); // to split headers and bodh
}


void prepare_response(int client, char file[]) {
	FILE *op = fopen(file, "rb");
	FILE *dup_op = op;

	if (op == 0)
		op = fopen("404.html", "r");

	fseek(op, 0, SEEK_END);

	long fsize = ftell(op);
	unsigned char content[fsize+1]; // an extra byte for null byte 

	prepare_headers(dup_op, file, sizeof(content)-1);

	fseek(op, 0, SEEK_SET);
	fread(content, 1, fsize, op);
	fclose(op);

	content[fsize] = '\0'; // setting null byte to extra byte for terminating string

	write(client, resp_headers, strlen(resp_headers));
	write(client, content, fsize);
}

void *process_request(void*arg) {
	struct process *my_proc = (struct process*)my_proc;
	int fd_client = my_proc->fd_client;
	char req[] = " ";
	char path[] = " ";

	strcpy(req, (*my_proc).request);
	strcpy(path, (*my_proc).path);
	char *token = strtok(req, "\n\r");

	char header[HEADER_SZ];
	strcpy(header, token);

	char *tokens = strtok(header, " ");
	
	char method[10];
	strcpy(method, tokens);

	tokens = strtok(NULL, " ");
	char file[strlen(tokens)];
	sprintf(file, "%s%s", path, tokens);

	prepare_response(fd_client, file);

	close(fd_client);
}

int main(int argc, char const *argv[])
{
	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_len = sizeof(client_addr);
	int fdimg, fd_server, fd_client;
	char req[req_len], path[100] = ".";
	int on = 1, port;
	 struct process *proc;

	fd_server = socket(AF_INET, SOCK_STREAM, 0);

	if (argc < 2) {
		printf("Port error! Please specify a port\n");
		exit(1);
	}

	port = atoi(argv[1]);

	if (argc > 2)
		strcpy(path, argv[2]);

	if(fd_server < 0) {
		perror("socket");
		exit(1);
	}
	
	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR,&on, sizeof(int));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
	{
		printf("error\n");
		perror("bind");
		close(fd_server);
		exit(1);
	}

	int conn_limit = 10;
	if(listen(fd_server, conn_limit) ==-1)
	{
		perror("listen");
		close(fd_server);
		exit(1);
	}

	printf("Server started\n\n");

	while(1)
	{
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
		if(fd_client == -1) {
			perror("connection error");
			continue;
		}

		if(!fork()) {
			close(fd_server);
			memset(req, 0, 2048);
			read(fd_client, req, 2047);

			printf("%s\n", req);
			//process_request(req, fd_client, path);
			pthread_t id;
			proc = malloc(sizeof(struct process));
			(*proc).request = req;
			proc->fd_client = fd_client;
			(*proc).path = path;
			pthread_create(&id, NULL,&process_request, (void*) proc);
			pthread_join(id, NULL);
					
			// printf("closing\n");
            exit(0);		
		}
		close(fd_client);
	}

	return 0;
}