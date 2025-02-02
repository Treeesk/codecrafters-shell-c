#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
      else 
        printf("%s: not found\n", &input[5]);
    }
    else
      printf("%s: command not found\n", input);
  }
  return 0;
}
