#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

char webpage[] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n";

int main(int argc, char const *argv[])
{
	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_len = sizeof(client_addr);
	int fdimg, fd_server, fd_client;
	char buf[2048];
	int on =1;
	int port;

	fd_server = socket(AF_INET, SOCK_STREAM, 0);

	if (argc < 2)
	{
		printf("Port error! Please specify a port\n");
		exit(1);
	}

	port = atoi(argv[1]);
	
	if(fd_server < 0)
	{
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

	printf("Server started\n");
	while(1)
	{
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
		if(fd_client == -1)
		{
			perror("connection error");
			continue;
		}

		// printf("got client connection\n");
		if(!fork())
		{
			// close(fd_server);
			// memset(buf, 0, 2048);
			// read(fd_client, buf, 2047);

			// // printf("%s\n", buf);
			// if(strncmp(buf, "GET /doctest.png", 16))
			// {
			// 	// int webfd = open("index.html", O_RDONLY);

			// 	// write(fd_client, webpage, sizeof(webpage) -1);
			// }
			
			// close(fd_client);
			
			// printf("closing\n");
			
			// exit(0);		
		}
		close(fd_client);
	}

	return 0;
}