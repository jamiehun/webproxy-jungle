#include "csapp.h"
#include "echo.c"


void echo(int connfd);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */ // 구조체 변수 선언 clientaddr은 Accept로 보내짐
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]); // 듣기 식별자를 오픈하고 리턴하는 도움 함수
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        
        /* Accept 함수는 연결요청이 듣기 식별자 listenfd에 도달하기를 기다리고, addr 내의 클라이언트의 소켓 주소를 채우고,
         * Unix I/O 함수들을 사용해서 클라이언트와 통신하기 위해 사용될 수 있는 연결 식별자를 리턴 
         * accept가 리턴되기 전에 clientaddr에는 연결의 다른 쪽 끝 클라이언트 소켓 주소로 채워짐 */
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd); // 
        Close(connfd);

    }

}