/*
 * client.c: A very, very primitive HTTP client.
 * 
 * To run, try: 
 *      ./client www.cs.technion.ac.il 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * HW3: For testing your server, you will want to modify this client.  
 * For example:
 * 
 * You may want to make this multi-threaded so that you can 
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs; 
 * you may want to get more URIs from the command line 
 * or read the list from a file. 
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include "segel.h"

/*
 * Send an HTTP request for the specified file
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}

/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];
  int length = 0;
  int n;

  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

int main(int argc, char *argv[])
{
  char *host, *filename;
  int port;
  int clientfd;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  port = atoi(argv[2]);
  filename = argv[3];
  pid_t pids[10];
  pid_t pid;
  for (int i = 0; i < 10 ; i ++){
      pid = fork();
      if(pid > 0 ){
          pids[i] = pid;
          break;
      }
  }
  /* Open a single connection to the specified host and port */
  clientfd = Open_clientfd(host, port);

  clientSend(clientfd, filename);
  clientPrint(clientfd);

  Close(clientfd);

  if(pid == 0){
      for (int i = 0 ; i < 10 ; i++){
          waitpid(pids[i],NULL, 0);
      }
  }
  exit(0);
}


/****************************ORI IMPLEMANTATION*************************************/
//#include <stdio.h>
//#include <stdlib.h>
//#include <curl/curl.h>
//
//void make_request(const char* hostname, int port, const char* filename) {
//    CURL *curl;
//    CURLcode res;
//    char url[256];
//
//    snprintf(url, sizeof(url), "http://%s:%d/%s", hostname, port, filename);
//
//    curl = curl_easy_init();
//    if(curl) {
//        curl_easy_setopt(curl, CURLOPT_URL, url);
//        res = curl_easy_perform(curl);
//        if(res != CURLE_OK)
//            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
//
//        curl_easy_cleanup(curl);
//    }
//}
//
//int main(int argc, char *argv[]) {
//    if (argc != 4) {
//        fprintf(stderr, "Usage: %s <hostname> <port> <filename>\n", argv[0]);
//        return 1;
//    }
//
//    const char* hostname = argv[1];
//    int port = atoi(argv[2]);
//    const char* filename = argv[3];
//
//    make_request(hostname, port, filename);
//
//    return 0;
//}