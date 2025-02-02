#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
  char input[100];
  while (1){
    int i = 0;
    printf("$ ");
    // Flush after every printf
    setbuf(stdout, NULL);

    // Wait for user input
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';
    if (strcmp(input, "exit 0") == 0)
      exit(0);
    if (strncmp(input, "echo ", 5) == 0) 
      printf("%s\n", &input[5]);
    else
      printf("%s: command not found\n", input);
  }
  return 0;
}
