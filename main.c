#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define PORT 3000
#define OK_TEMPLATE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s"
#define NO_METHOD_TEMPLATE "HTTP/1.1 405 Method Not Allowed\r\n\r\n"
#define NOT_FOUND_TEMPLATE "HTTP/1.1 404 Not Found\r\n\r\n"

typedef void (*route_func_ptr)(int, char*);

typedef struct node {
    char url[1024];
    route_func_ptr func_ptr;
    struct node *next;
} RouterNode; 

extern void route_index(int, char*);
extern void route_another(int, char*);
extern void route_static(int, char*);

char* dupstr(char* str);
void handle_sigint(int sig);
int start_socket();
void handle_error(int val, const char* errorMsg);
void append_route(RouterNode **head, const char* url, route_func_ptr func_ptr);
RouterNode* get_route(RouterNode *head, const char* url);

int sockfd;
int client_sockfd;

int main(void) {
  signal(SIGINT, handle_sigint);
  sockfd = start_socket();
  handle_error(sockfd, "socket");

  RouterNode *head = NULL;
  append_route(&head, "/new/", route_another);
  append_route(&head, "/static/", route_static);
  append_route(&head, "/", route_index);

  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
    perror("bind");
    exit(1);
  }

  handle_error(listen(sockfd, 10), "listen"); 

  while(1) {
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr);
    client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
    handle_error(client_sockfd, "accept");

    char request_buf[1024] = {0};
    ssize_t request_size = read(client_sockfd, request_buf, sizeof(request_buf));
    handle_error(request_size, "read");
    printf("%s\n", request_buf);
    
    char method[16] = {0};
    char url[1024]= {0};
    sscanf(request_buf, "%s %s", method, url);

    if(strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0) {  
      const char* response = NO_METHOD_TEMPLATE;
      write(client_sockfd, response, strlen(response));
      close(client_sockfd);
      continue;
    }

    if (strcmp(method, "GET") == 0) {
      RouterNode* route = get_route(head, url);
      
      if (route==NULL) {
        printf("404 Error Not Found!\n");
        const char* response = NOT_FOUND_TEMPLATE;
        write(client_sockfd, response, strlen(response));
        close(client_sockfd);
        continue;
      }
      
      int size = strlen(route->url);
      if (strlen(url) == size) {
        (*route->func_ptr)(client_sockfd, "");
      } else {
        char *rest = dupstr(url+size);
        (*route->func_ptr)(client_sockfd, rest);
        free(rest);
      }
    }
    close(client_sockfd);
  }

  close(sockfd);
  return 0;
}

void handle_sigint(int sig) {
  printf("\nExiting...\n");
  close(sockfd);
  close(client_sockfd);
  exit(1);
}

int start_socket() {
  return socket(AF_INET, SOCK_STREAM, 0);
}

void handle_error(int val, const char* errorMsg) {
  if(val < 0) {
    perror(errorMsg);
    exit(1);
  }
} 

void append_route(RouterNode **head, const char* url, route_func_ptr func_ptr) {
  RouterNode *elem = malloc(sizeof(RouterNode));
  elem->next = NULL;
  strcpy(elem->url, url);
  elem->func_ptr = func_ptr;
  if(*head) {
    RouterNode *ptr = *head;
    while(ptr->next) {
      ptr = ptr->next;
    }
    ptr->next = elem;
  } else {
    *head = elem;
  }
}

RouterNode* get_route(RouterNode *head, const char* url) {
  RouterNode *ptr = head;
  while(ptr) {
    if(strncmp(url, ptr->url, strlen(ptr->url)) == 0) {
      return ptr;
    }
    ptr = ptr->next;
  }
  return NULL;
}

char* dupstr(char* str){
  if (str==NULL) {return NULL;}
  char* temp = malloc(sizeof(str)+1);
  strcpy(temp, str);
  return temp;
}
