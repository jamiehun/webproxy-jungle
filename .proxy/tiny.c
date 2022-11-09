/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 * 
 * TINY는 반복실행서버로 명령줄에서 넘겨받은 포트로의 연결요청을 들음
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  // argc는 카운트를 의미 (ex_./tiny 5000에서 ./tiny가 0 5000이 1)
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]); //
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]); // 서버는 연결 요청을 받을 준비가 된 듣기 식별자를 생성
  while (1) {
    clientlen = sizeof(clientaddr); // clientaddr의 주소를 알려줌
    
    /* 클라이언트로부터의 연결 요청이 듣기식별자 listenfd에 도달 
     * -> 그 후 addr 내의 소켓주소 채우기 
     * -> 클라이언트와 통신하기 위해 사용될 수 있는 connected descriptor */ 
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept // 듣기 소켓을 오픈
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0); // 구조체 -> 이름 서비스번호 등
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit 트랜잭션을 수행
    Close(connfd);  // line:netp:tiny:close 자신쪽의 연결 끝을 닫음
  }
}

/* 한 개의 HTTP 트랜잭션을 처리 */
void doit(int fd) // 여기서 fd는 위의 연결식별자 (connfd)
{
  int is_static;
  struct stat sbuf; // 이것만 해줘도 안에 있는 구조체가 다 나옴?
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd); // 연결식별자와 rio 구조체(내부버퍼)를 연결 

  /* 요청라인을 읽고 분석 */
  Rio_readlineb(&rio, buf, MAXLINE); // 텍스트 줄을 읽음 (rio -> buf 복사)
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  /* Tiny는 Get 메소드만 지원하며 만일 클라이언트가 다른 메소드를 요청하면 에러메시지를 보냄*/
  if(strcasecmp(method, "GET")){ // 다르면 0이 아니고 같으면 0 (False)
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio); // 다른 헤더 요청들은 무시

  /* Parse URI from GET request */
  /* URI를 파일 이름과 비어 있을 수도 있는 CGI 인자 스트링으로 분석하고 
     요청이 정적 또는 동적 컨텐츠 위한 것인지 나타내는 플래그를 설정 */
  is_static = parse_uri(uri, filename, cgiargs); // 여기에서 filename은 비어있고 밑의 parse_uri에서 채워줌

  /* 만일 이 파일이 디스크 상에 있지 않으면 에러메시지를 즉시 클라이언트에게 보내고 리턴 */
  if (stat(filename, &sbuf) < 0){ // stat는 filename이 sbuf 안에 있는지 봄 => 에러면 -1, 성공이면 0을 리턴
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if (is_static) { /* Serve static content */
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){ /* 정규파일인지, 혹은 사용자가 파일을 읽을 수 있는지*/
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size); /* 정적 컨텐츠를 클라이언트에게 제공 */
  }
  else{ /* Serve dynamic content */
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){  /* 정규파일인지, 혹은 사용자가 파일을 실행할 수 있는지 */
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program"); 
      return;
    }
    serve_dynamic(fd, filename, cgiargs); /* 동적 컨텐츠를 서버에게 제공 */

  }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
  char buf[MAXLINE], body[MAXLINE];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE); // rp -> buf로
  while(strcmp(buf, "\r\n")){      
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf); // buf를 복사, 프린트만 함
  }
  return;
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  if (!strstr(uri, "cgi-bin")) { /*Static content*/ // str => pointer to "cgi-bin", does not appear => returns NULL;
    strcpy(cgiargs, "");  // cgi 인자 string을 지움
    strcpy(filename, "."); // uri를 ./index.html과 같은 리눅스 상대 경로 이름으로 변환
    strcat(filename, uri);
    if (uri[strlen(uri)-1] == '/') // 만일 uri가 '/'로 끝난다면
      strcat(filename, "adder.html"); // 파일 이름 뒤에 adder.html을 넣어줌
    return 1;
  }
  else{ /* Dynamic content */
    ptr = index(uri, '?'); // '?'에 해당하는 index를 찾음
    if (ptr) {             // ptr 있으면
      strcpy(cgiargs, ptr+1); // cgi 인자에 ptr + 1을 넣음
      *ptr = '\0';            // '?'에 해당하는 값은 '\0'으로 바꿈
    }
    else
      strcpy(cgiargs, "");    // ptr 없으면 cgi 인자 공백으로 바꿈
    strcpy(filename, ".");     // 위와 같이 filename에 .과 uri를 넣음
    strcat(filename, uri);
    return 0;
  }
}

/*
 * serve_static : 
 */

void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);
  /* 응답 줄과 응답 헤더를 보냄 */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype); // 빈줄 한개가 헤더를 종료하고 있음
  Rio_writen(fd, buf, strlen(buf)); // buf -> fd로 보냄, 여기에서 fd는 연결식별자 connfd

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0); // 파일을 오픈하고 식별자를 받아옴 
  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // mmap함수는 요청한 파일을 가상 메모리 영역으로 매핑
  srcp = (char *)malloc(filesize);
  Rio_readn(srcfd, srcp, filesize); // srcfd -> srcp로 보냄 
  Close(srcfd); // 파일을 닫아줌 
  Rio_writen(fd, srcp, filesize); //srcp에 있는 filesize 바이트를 클라이언트 연결식별자로 보냄
  free(srcp); // srcp를 반환 (메모리 누수를 막음)
  // Munmap(srcp, filesize); // Mmap으로 가상메모리 영역에 매핑한 메모리 주소를 반환함

}

/*
 * get_filetype - Derive file type from filename
 */

// strstr은 string1에서 string2의 첫번째 표시를 찾으며
// return은 string1에서 string2의 첫번째 표시 시작 위치에 대한 포인터를 리턴,
// 없으면 NULL을 리턴
void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}

/*
 * serve_dynamic - 
 */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf)); // buf -> fd
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));  

  if (Fork() == 0) { /* Child */ // 자식을 fork함
      /* Real server would set all CGI vars here */
      setenv("QUERY_STRING", cgiargs, 1); // Query_STRING 환경변수를 요청 URI의 CGI 인자들로 반환
      Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */ // 자식은 자식의 표준 출력을 연결파일 식별자로 재지정
      // #define	STDOUT_FILENO	1	/* Standard output.  */
      /* CGI 프로그램이 자식 컨텍스트에서 실행되기 때문에 execve 함수를 호출하기 전에 존재하던 열린 파일들과
       * 환경변수들에도 동일하게 접근 가능 
       * 그래서 CGI 프로그램이 표준 출력에 쓰는 모든 것은 직접 클라이언트 프로세스로 부모 프로세스의 어떤 간섭도 없이 전달됨
       */ 
      Execve(filename, emptylist, environ); /* Run CGI Program*/
  }
  // 부모는 자식이 종료되어 정리되는 것을 기다리기 위해 wait 함수에서 블록됨
  Wait(NULL); /* Parent waits for and reaps child */

}
