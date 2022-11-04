#include "csapp.h"

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd); // rio라는 변수 주소에 clientfd의 값을 넣음
    while((n = Rio_readlineb(&rio, buf, MAXLINE))!= 0){  // rio_readlineb - Robustly read a text line (buffered)
        printf("server received %d bytes \n", (int)n);
        Rio_writen(connfd, buf, n);                     // rio_writen - Robustly write n bytes (unbuffered) 
    }
}