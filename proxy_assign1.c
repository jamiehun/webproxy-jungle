/* 웹프록시서버 만들기 과제 1번 */

#include <stdio.h>
#include "queue.c"
#include "sbuf.h"

#define NTHREADS 4
#define SBUFSIZE 16

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

#include "csapp.h"
#include <stdlib.h>

void doit(int fd);
void send_request(char *uri, int fd);
void *thread(void *vargp);

char dest[MAXLINE];
Queue queue; // 큐에 대한 구조체 설정
sbuf_t sbuf;
sem_t mutex;


int main(int argc, char **argv) 
{
  int listenfd, *connfdp;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t tid;

  
  
  InitQueue(&queue);    // 큐 초기화
  sem_init(&mutex,0,1); // 세마포어 초기화

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]); // Listenfd 듣기 식별자 할당 (telnet <-> proxy)

  sbuf_init(&sbuf, SBUFSIZE); // 세마포어 활용을 위한 sbuf init
  
  /* Create worker threads */
  int i = 0;
  for (i = 0; i < NTHREADS; i++) 
  {
    Pthread_create(&tid, NULL, thread, NULL); //thread 함수를 불러들이는 함수
  }

  while (1) {
    clientlen = sizeof(clientaddr);
    connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 요청받은만큼 연결식별자 만들기
    
    /* Insert connfd in buffer */
    sbuf_insert(&sbuf, connfdp); 
  }

}

void *thread(void *vargp)
{
  Pthread_detach(pthread_self());
  while (1) {
    /* Remove connfd from buffer */
    /* 버퍼에 connfd가 있는지 보고 connfd를 삭제 후 스레드에 연결 */
    int connfd = sbuf_remove(&sbuf); // 세마포어 V 연산
  // ----------------- start of proxy server request ------------------
    /* Service client */
    doit(connfd);
    Close(connfd);
  }
  
  // ----------------- end of entire request ------------------
  return NULL;
}


// fd : telnet - proxy
void doit(int fd)
{
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;
  

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);

  printf("Request line:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  /* p, v 연산으로 cache에 대한 다중 접근 잠그기 */
  P(&mutex);
  Node *cache = queue.front;
  while (cache != NULL){
    
    if(strcmp(cache->request_line, uri) == 0){
      // --------------------cache hit!!
      Rio_writen(fd, cache->response, strlen(cache->response));
      return;
    }
    cache = cache->next;
  }
  V(&mutex);

  send_request(&uri, fd);
}

void send_request(char *uri, int fd){

  int clientfd;
  char buf[MAXLINE], proxy_res[MAX_OBJECT_SIZE], tmp[MAX_OBJECT_SIZE], tmp2[MAX_OBJECT_SIZE], port[MAX_OBJECT_SIZE], new_uri[MAX_OBJECT_SIZE];
  rio_t rio;
  char *p, *q;

  // uri parsing
  sscanf(strstr(uri, "http://"), "http://%s", tmp);
  if((p = strchr(tmp, ':')) != NULL){
    *p = '\0';
    sscanf(tmp, "%s", dest);
    sscanf(p+1, "%s", tmp2);

    q = strchr(tmp2, '/');
    *q = '\0';
    sscanf(tmp2, "%s", port);
    *q = '/';
    sscanf(q, "%s", new_uri);
    
  }

  // proxy - tiny
  clientfd = Open_clientfd(dest, port);

  // request header
  Rio_readinitb(&rio, clientfd);
  sprintf(buf, "GET %s HTTP/1.0\r\n\r\n", new_uri);


  Rio_writen(clientfd, buf, strlen(buf));
  printf("%s", buf);
  Rio_readnb(&rio, proxy_res, MAX_OBJECT_SIZE);

  Rio_writen(fd, proxy_res, MAX_OBJECT_SIZE);
  Close(clientfd);

  // p, v 연산으로 queue에 대한 다중 접근 잠그기
  P(&mutex);
  if (queue.count > 10){
    Dequeue(&queue);
  // ------------------dequeue
  }

  printf("%s\n", (proxy_res));
  Enqueue(&queue, uri, &proxy_res);
  // ----------------enqueue
  V(&mutex);
}
