#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>

void parse_input(char *inp, char **argv, int *argc, char **outf) {
  char *start = inp;
  short int in_quotes = 0;
  char type_quotes = 0;
  *outf = NULL; // Инициализируем outf как NULL
  short int check = 0;

  for (int i = 0; inp[i]; i++) {
    if (inp[i] == '\\' && ((type_quotes == '\"' && (inp[i + 1] == '\\' || inp[i + 1] == '\"')) || (type_quotes == '\'' && (inp[i + 1] == '\\' || inp[i + 1] == '\'' || inp[i + 1] == '\"')))){
      memmove(&inp[i], &inp[i + 1], strlen(&inp[i + 1]) + 1); // strlen(inp[i + 1]) + 1 inp[i + 1]-начинаем длину считать со следующего символа, +1 - для \0
      check = 1;
      continue;
    }

    if (inp[i] == '1' && inp[i + 1] == '>' && !in_quotes) { // Обработка перенаправления вывода 1>
          inp[i] = '\0'; // Завершаем текущий аргумент
          *outf = &inp[i + 2]; // Указываем на начало имени файла
          while (**outf == ' ') { // Пропускаем пробелы
              (*outf)++;
          }
          break; // Завершаем разбор, так как дальше идет имя файла
    }
    else if (inp[i] == '>' && !in_quotes) { // Обработка перенаправления вывода >
          inp[i] = '\0'; // Завершаем текущий аргумент
          *outf = &inp[i + 1]; // Указываем на начало имени файла
          while (**outf == ' ') { // Пропускаем пробелы
              (*outf)++;
          }
          break; // Завершаем разбор, так как дальше идет имя файла
    }

    else if ((inp[i] == '\'' || inp[i] == '\"') && !in_quotes) { // Обработка кавычек
          in_quotes = 1;
          start = &inp[i + 1]; // Начинаем новый аргумент после кавычки
          type_quotes = inp[i];
    } 
    else if (inp[i] == type_quotes && in_quotes) { // Завершение кавычек
      if (i > 0 && inp[i - 1] != '\\'){
          in_quotes = 0;
          inp[check ? i - 1 : i] = '\0'; // Завершаем текущий аргумент
          argv[(*argc)++] = start;
          start = NULL; // Сбрасываем указатель на начало аргумента
      }
    } 
    else if (inp[i] == ' ' && !in_quotes) { // Обработка пробелов
        if (start != NULL) {
          inp[i] = '\0'; // Завершаем текущий аргумент
          argv[(*argc)++] = start; // Добавляем аргумент в массив
          start = NULL; // Сбрасываем указатель на начало аргумента
          }
        while (inp[i + 1] == ' ')
          i++;
      }
    else if (start == NULL) { // Начало нового аргумента
        start = &inp[i];
    }
  }

  if (start != NULL) { // Последний аргумент
      argv[(*argc)++] = start;
  }
  argv[*argc] = NULL; // Завершаем массив аргументов NULL
}

void fork_func(char *full_path, char **argv, char *outf){
  pid_t pid = fork();
  if (pid == 0) {
    if (outf){
      int flags = O_WRONLY | O_CREAT | O_TRUNC;
      int fd = open(outf, flags, 0666);
      if (fd == -1){
        perror("open");
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }
    execv(full_path, argv);
    perror("execv"); // если ошибка в Execv
    exit(1);
  } else if (pid < 0)
    perror("fork");
  else {
    int status;
    waitpid(pid, &status, 0);
  }
}

char *check_path(char *f){
  char *path_check = getenv("PATH");
  if (path_check == NULL)
    return NULL;

  char *path_copy = strdup(path_check);
  char *dir = strtok(path_copy, ":");
  static char full_path[1024];

  while (dir != NULL){
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, f);
    if (access(full_path, F_OK) == 0){
      free(path_copy);
      return full_path;
    }
    dir = strtok(NULL, ":");
  }
  free(path_copy);
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
    else if (strncmp(input, "type ", 5) == 0){
      if (strcmp(input, "type type") == 0)
        printf("type is a shell builtin\n");
      else if (strcmp(input, "type exit") == 0)
        printf("exit is a shell builtin\n");
      else if (strcmp(input, "type echo") == 0)
        printf("echo is a shell builtin\n");
      else if (strcmp(input, "type pwd") == 0)
        printf("pwd is a shell builtin\n");
      else if (strcmp(input, "type cd") == 0)
        printf("cd is a shell builtin\n");
      else if (check_path(&input[5]))
        printf("%s is %s\n", &input[5], check_path(&input[5]));
      else {
          printf("%s: not found\n", &input[5]);
      }
    }
    else if(strcmp(input, "pwd") == 0){
      char *pat = realpath(".", NULL);
      if (pat){
        printf("%s\n", pat);
      }
      else{
        printf("Error");
      }
    }
    else if (strncmp(input, "cd ", 3) == 0){
      int result;
      if (input[3] == '~'){
        char *dir = getenv("HOME");
        result = chdir(dir);
      }
      else
        result = chdir(&input[3]);
      if (result == -1)
        printf("cd: %s: No such file or directory\n", &input[3]);
    }
    else{
      char *argv[10];
      int argc = 0;
      char *output_file = NULL;
      parse_input(input, argv, &argc, &output_file);
      char *pth = check_path(argv[0]); // возвращаю полный путь до команды например cat, а затем применяю эту команду к аргументам argv
      if (pth != NULL)
        fork_func(pth, argv, output_file); 
      else {
        printf("%s\n", argv[0]);
        printf("%s: command not found\n", argv[0]); 
      }
    }
  }
  return 0;
}
