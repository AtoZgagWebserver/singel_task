#include "threadfunc.h"
#include "httpfunc.h"

#define MAX_DATA_SIZE 1024

extern struct QuestionList *question;
extern TaskQueue queue;

void init_queue(TaskQueue *q) {
    q->front = q->rear = q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

// 작업 큐에 작업 추가
void enqueue(TaskQueue *q, int *client_socket) {
    pthread_mutex_lock(&q->lock);

    while (q->count == QUEUE_SIZE) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    q->client_sockets[q->rear] = client_socket;  // 포인터 추가
    q->rear = (q->rear + 1) % QUEUE_SIZE;
    q->count++;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}


int* dequeue(TaskQueue *q) {
    pthread_mutex_lock(&q->lock);

    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    int* client_socket = q->client_sockets[q->front];  // 포인터 반환
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->count--;

    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);

    return client_socket;
}


void *client() {
    while(1){
        int *ns = dequeue(&queue);
        char buf[MAX_DATA_SIZE];  // 수신 버퍼
        ssize_t n;
        char data[MAX_DATA_SIZE];  // 요청 데이터를 저장할 배열
        int data_len = 0;

        // 클라이언트로부터 HTTP 요청을 수신
        n = recv(*ns, data, MAX_DATA_SIZE - 1, 0);
        data_len=strlen(data);

        if(n>0){
            data[data_len] = '\0';

            struct HTTPRequest http_request;  // HTTPRequest로 일관되게 사용
            memset(&http_request, 0, sizeof(struct HTTPRequest));

            parse_http_request(data, &http_request);

            //요청에 따라 어떻게 처리할지
            if (strcmp(http_request.method, "GET") == 0) { //GET 요청의 경우
                if(strcmp(http_request.path,"/quiz")==0){
                    send_quiz(*ns);
                }
                else{
                    char file_path[512];
                    snprintf(file_path, sizeof(file_path), "./rsc/html/%s", http_request.path[0] == '/' ? http_request.path + 1 : http_request.path);
                    send_file_content(*ns, file_path);
                }
            }
            if (strcmp(http_request.method, "POST") == 0) { 
                if(1){
                    const char *not_found_response = 
                        "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 13\r\n"
                        "\r\n"
                        "404 Not Found";
                    send(*ns, not_found_response, strlen(not_found_response), 0);
                }
            }
        }

        close(*ns);
        free(ns);
    }
    return NULL;
}