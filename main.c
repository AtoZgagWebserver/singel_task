#include "lib/headerlist.h"
#include "lib/readdata.h"
#include "lib/threadfunc.h"
#include "lib/httpfunc.h"

#define THREAD_POOL_SIZE 10

TaskQueue queue;

struct QuestionList *question;

int main(int argc, char *argv[]){
    int PORTNUM = atoi(argv[1]);

	struct sockaddr_in sin,cli;
	int sd, clientlen = sizeof(cli);


	memset((char*)&sin,'\0',sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_port=htons(PORTNUM);
	sin.sin_addr.s_addr=inet_addr("0.0.0.0");
	
	if((sd = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("soket");
		exit(1);
	}

    int optval = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if(bind(sd,(struct sockaddr *)&sin, sizeof(sin))){
			perror("bind");
			exit(1);
	}
	
	if(listen(sd,20000)){
		perror("listen");
		exit(1);
	}

    printf("%d 포트 대기\n",PORTNUM);

    init_queue(&queue);

    question=read_gag();

    printf("데이터 로드 완료\n");


    pthread_t thread_pool[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&thread_pool[i], NULL, client, NULL) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    printf("thread_pool done\n");

    while (1) {
        // 클라이언트 연결을 기다립니다.
        int *client_socket = malloc(sizeof(int));  // 클라이언트 소켓에 대한 메모리 할당
        if (!client_socket) {
            perror("malloc failed");
            continue;
        }

        *client_socket = accept(sd, (struct sockaddr *)&cli, &clientlen);  // 클라이언트 연결 수락
        if (*client_socket == -1) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        // 클라이언트 소켓을 큐에 추가
        enqueue(&queue, client_socket);  // 큐에 클라이언트 소켓을 넣습니다.
    }

	close(sd);
	return 0;
}