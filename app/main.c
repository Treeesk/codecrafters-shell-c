#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
  char input[100];
  char check_for_com[10];
  while (1){
    int i = 0;
    // Uncomment this block to pass the first stage
    printf("$ ");
    // Flush after every printf
    setbuf(stdout, NULL);

    // Wait for user input
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';
    if (strcmp(input, "exit 0") == 0)
      exit(0);
    for (i = 0; input[0] == 'e' && input[i] != ' '; i++){
      check_for_com[i] = input[i];
    }
    check_for_com[i] = '\0';
    if (strcmp(check_for_com, "echo") == 0)
      printf("%s\n", &input[i]);
    else
      printf("%s: command not found\n", input);
  }
  return 0;
}
