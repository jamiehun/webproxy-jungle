/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1=0, n2=0, n3=0;
  char *num1, *num2;

  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL ){ // buf = num=55
    p = strchr(buf, '&'); // 스트링에서 문자의 첫번째 표시에 대한 포인터 리턴값 반환 => num2 // (p + 1 = num2 = 5)
    *p = '\0';
    num1 = strchr(buf, '=');
    num2 = strchr((p+1), '=');
    // strcpy(arg1, buf); // string2를 string1에서 지정한 위치로 복사 (복사된 스트링에 대한 포인터 리턴)
    // strcpy(arg2, p+1); 
    // n1 = atoi(arg1);
    // n2 = atoi(arg2);
    n1 = atoi(num1+1);
    n2 = atoi(num2+1);
    n3 = n1 + n2;
  }

  /* Make the response body */
  sprintf(content, "QUERY_STRING=%s", buf); 
  // sprintf(content, "QUERY_STRING2=%s ", p+1);
  sprintf(content, "%sWelcome to add.com: "); 
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sThe correct answer is: %d + %d = %d\r\n<p>",
          content, n1, n2, n3);
  sprintf(content, "%sThanks for visiting~~!\r\n", content);
  

  /* Generate the HTTP response */
  /* Header 부분 */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  
  /* Body 부분 */
  printf("%s", content);
  fflush(stdout);
  exit(0);
}
/* $end adder */
