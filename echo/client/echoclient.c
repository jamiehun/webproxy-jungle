#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd; // 클라이언트 식별자
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) { // 3인 이유는?
        fprintf(stderr, "usage: %s <host> <port> \n", argv[0]); // stderr이라는 파일에 해당 내용을 적음
        exit(0);
    }
    host = argv[1]; // 호스트 번호 인덱싱
    port = argv[2]; // 포트 번호 인덱싱

    clientfd = Open_clientfd(host, port);  // clientfd는 클라이언트 소켓 식별자
    Rio_readinitb(&rio, clientfd);         // 식별자 fd를 &rio 주소에 위치한 rio_t 타입의 읽기 버퍼와 연결
                                           // rio_readinitb - Associate a descriptor with a read buffer and reset buffer
                                           //

    while (Fgets(buf, MAXLINE, stdin) != NULL){  // 글을 읽어 들여 NULL 값이 뜨지 않을 때까지 읽음
        Rio_writen(clientfd, buf, strlen(buf));  // rio_writen - Robustly write n bytes (unbuffered) buf -> clientfd로 보냄
        // 이 사이에 무슨 일이? rio에 저장되는 것이 있나?
        Rio_readlineb(&rio, buf, MAXLINE);       // rio_readlineb - Robustly read a text line (buffered) rio에서 읽고 -> buf 복사
        Fputs(buf, stdout);
    }
    Close(clientfd);
    exit(0);
}