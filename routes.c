#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define INDEX_FILE "index.html"
#define ANOTHER_FILE "another.html"

#define OK_TEMPLATE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s"
#define OK_REQUEST_TEMPLATE "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n%s"
#define NO_METHOD_TEMPLATE "HTTP/1.1 405 Method Not Allowed\r\n\r\n"
#define NOT_FOUND_TEMPLATE "HTTP/1.1 404 Not Found\r\n\r\n"

bool handle_file_error(int client_sockfd, FILE *file) {
  if(!file) {
    const char* error = NOT_FOUND_TEMPLATE;    
    write(client_sockfd, error, strlen(error));
    return true;
  }
  return false;
}

int get_file_size(FILE* file) {
  fseek(file, 0, SEEK_END);
  int file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  return file_size;
}

char* get_file_contents(FILE *file, int file_size) {
  char *file_contents = malloc(file_size);
  fread(file_contents, 1, file_size, file);
  return file_contents;
}

void route_index(int client_sockfd, char* rest) {
  const char* filepath = "./" INDEX_FILE;
  FILE *file = fopen(filepath, "r");
  if (handle_file_error(client_sockfd, file)) return;
  
  int file_size = get_file_size(file);
  char *file_contents = get_file_contents(file, file_size);

  char response_buf[2048] = {0};
  sprintf(response_buf, OK_TEMPLATE, (int) file_size, file_contents);
  write(client_sockfd, response_buf, sizeof(response_buf));

  free(file_contents);
  fclose(file);
}

void route_another(int client_sockfd, char* rest) {
  const char* filepath = "./" ANOTHER_FILE;
  FILE *file = fopen(filepath, "r");
  if (handle_file_error(client_sockfd, file)) return;
  
  int file_size = get_file_size(file);
  char *file_contents = get_file_contents(file, file_size);

  char response_buf[2048] = {0};
  sprintf(response_buf, OK_TEMPLATE, (int) file_size, file_contents);
  write(client_sockfd, response_buf, sizeof(response_buf));

  free(file_contents);
  fclose(file);
}

void route_static(int client_sockfd, char* rest) {
  char* filepath = (char *) malloc((3+strlen(rest))*sizeof(char));
  strcpy(filepath, "./");
  strcat(filepath, rest);
  printf("Reqesting for %s\n", filepath);
  FILE *file = fopen(filepath, "r");
  if (handle_file_error(client_sockfd, file)) return;
  
  int file_size = get_file_size(file);
  char *file_contents = get_file_contents(file, file_size);

  char response_buf[2048] = {0};
  sprintf(response_buf, OK_REQUEST_TEMPLATE, "text/css", (int) file_size, file_contents);
  write(client_sockfd, response_buf, sizeof(response_buf));

  free(file_contents);
  fclose(file);
  free(filepath);
}
