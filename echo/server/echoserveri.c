#include "csapp.h"
#include "echo.c"


void echo(int connfd);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */ // 구조체 변수 선언 clientaddr은 Accept로 보내짐
    char client_hostname[MAXLINE], client_port[MAXLINE];

    // 오류 처리
    if (argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]); // 듣기 식별자를 오픈하고 리턴하는 도움 함수; 포트에 대해 연결요청을 받을 준비
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        
        /* Accept 함수는 연결요청이 듣기 식별자 listenfd에 도달하기를 기다리고, 
         * addr 내의 클라이언트의 소켓 주소를 채우고,
         * Unix I/O 함수들을 사용해서 클라이언트와 통신하기 위해 사용될 수 있는 연결 식별자를 리턴 */
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 연결식별자 clientfd <-> connfd
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0); // 소켓구조체 -> 호스트이름, 서비스이름
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd); // 
        Close(connfd);

    }

}