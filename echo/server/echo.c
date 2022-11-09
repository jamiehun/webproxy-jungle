void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd); // connfd 연결 할당자를 rio_t 타입의 읽기버퍼와 연결
    while((n = Rio_readlineb(&rio, buf, MAXLINE))!= 0){  // rio_readlineb - Robustly read a text line (buffered)
        printf("server received %d bytes \n", (int)n);
        Rio_writen(connfd, buf, n);                     // rio_writen - Robustly write n bytes (unbuffered) 
    }
}