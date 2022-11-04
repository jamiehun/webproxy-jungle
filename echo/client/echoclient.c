#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) { // 3인 이유는?
        fprintf(stderr, "usage: %s <host> <port> \n", argv[0]); // stderr이라는 파일에 해당 내용을 적음
        exit(0);
    }
    host = argv[1]; 
    port = argv[2];

    clientfd = Open_clientfd(host, port);  // clientfd는 클라이언트 소켓 식별자
    Rio_readinitb(&rio, clientfd);         // rio라는 변수 주소에 clientfd의 값을 넣음
                                           // 구조체 안에 소켓 식별자, 내부 버퍼 등을 삽입
                                           // rio_readinitb - Associate a descriptor with a read buffer and reset buffer

    while (Fgets(buf, MAXLINE, stdin) != NULL){  // 글을 읽어 들여 NULL 값이 뜨지 않을 때까지 읽음
        Rio_writen(clientfd, buf, strlen(buf));  // rio_writen - Robustly write n bytes (unbuffered) 
        Rio_readlineb(&rio, buf, MAXLINE);       // rio_readlineb - Robustly read a text line (buffered)
        Fputs(buf, stdout);
    }
    Close(clientfd);
    exit(0);
}