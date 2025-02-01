#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
  char input[100];
  while (1){
    // Uncomment this block to pass the first stage
    printf("$ ");
      // Flush after every printf
    setbuf(stdout, NULL);

    // Wait for user input
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';
    if (strcmp(input, "exit 0") == 0)
      break;
    printf("%s: command not found\n", input);
  }
  return 0;
}
