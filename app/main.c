#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char *check_path(char *f){
  char *path_check = getenv("PATH");
  if (path_check == NULL)
    return NULL;
  
  char *path_copy = strdup(path_check);
  char *dir = strtok(path_copy, ":");
  static char full_path[1024];

  while (dir != NULL){
    snprintf(full_path, sizeof(full_path), "%s%s", dir, f);
    if (access(path_copy, X_OK) == 0){
      return full_path;
    }
    dir = strtok(NULL, ":");
  }

  return NULL;
}

int main() {
  char input[100];
  while (1){
    setbuf(stdout, NULL);
    printf("$ ");
    
    // Wait for user input
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';
    if (strcmp(input, "exit 0") == 0)
      exit(0);
    else if (strncmp(input, "echo ", 5) == 0) 
      printf("%s\n", &input[5]);
    else if (strncmp(input, "type ", 5) == 0){
      if (strcmp(input, "type type") == 0)
        printf("type is a shell builtin\n");
      else if (strcmp(input, "type exit") == 0)
        printf("exit is a shell builtin\n");
      else if (strcmp(input, "type echo") == 0)
        printf("echo is a shell builtin\n");
      else if (check_path(&input[5]))
        printf("%s is %s", &input[5], check_path(&input[5]));
      else 
        printf("%s: not found\n", &input[5]);
    }
    else
      printf("%s: command not found\n", input);
  }
  return 0;
}
