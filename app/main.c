#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>


const char* data_autocompleting[] = {
  "echo",
  "exit",
};

void strrev(char* str)
{
    // if the string is empty
    if (!str) {
        return;
    }
    // pointer to start and end at the string
    int i = 0;
    int j = strlen(str) - 1;

    // reversing string
    while (i < j) {
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
        i++;
        j--;
    }
}

void parse_input(char *inp, char **argv, int *argc, char **outf, short int* err_f, short int* app) {
    char *start = inp;
    short int in_quotes = 0;
    char type_quotes = 0;
    *outf = NULL; // Инициализируем outf как NULL
    *err_f = 0;
    *app = 0;

    for (int i = 0; inp[i]; i++) {

        // Обработка перенаправления вывода
        if ((inp[i] == '1' && inp[i + 1] == '>' && inp[i + 2] == '>') || (inp[i] == '>' && inp[i + 1] == '>') && !in_quotes){
          inp[i] == '\0';
          if (inp[i] == '1') {
            *outf = &inp[i + 3];
          }
          else {
            *outf = &inp[i + 2];
          }
          *app = 1;
          while (**outf == ' ')
            (*outf)++;
          break;
        }

        else if (inp[i] == '1' && inp[i + 1] == '>' && !in_quotes) {
            inp[i] = '\0';
            *outf = &inp[i + 2];
            while (**outf == ' ') {
                (*outf)++;
            }
            break;
        }
        else if (inp[i] == '>' && !in_quotes) {
            inp[i] = '\0';
            *outf = &inp[i + 1];
            while (**outf == ' ') {
                (*outf)++;
            }
            break;
        }
        
        else if (inp[i] == '2' && inp[i + 1] == '>' && inp[i + 2] == '>' && !in_quotes){
          inp[i] == '\0';
          *outf = &inp[i + 3];
          *app = 1;
          *err_f = 1;
          while (**outf == ' ')
            (*outf)++;
          break;
        }

        else if (inp[i] == '2' && inp[i + 1] == '>' && !in_quotes){
          inp[i] = '\0';
          *outf = &inp[i + 2];
          while (**outf == ' ') {
              (*outf)++;
          }
          *err_f = 1;
          break;
        }

        // Обработка кавычек
        else if ((inp[i] == '\'' || inp[i] == '\"') && !in_quotes) {
            in_quotes = 1;
            start = &inp[i + 1];
            type_quotes = inp[i];
        } 
        else if (inp[i] == type_quotes && in_quotes ) { // Завершение кавычек
          if (inp[i + 1] == ' ' || inp[i + 1] == '\0'){
            in_quotes = 0;
            inp[i] = '\0'; // Завершаем текущий аргумент
            argv[(*argc)++] = start;
            start = NULL; // Сбрасываем указатель на начало аргумента
          }
          else if (inp[i + 1] == '\"' || inp[i + 1] == '\''){
            memmove(&inp[i], &inp[i + 2], strlen(&inp[i + 1]) + 1);
            continue;
          }
          else {
            memmove(&inp[i], &inp[i + 1], strlen(&inp[i + 1]) + 1);
            continue;
          }
        } 
        // Обработка пробелов
        else if (inp[i] == ' ' && !in_quotes) {
            if (start != NULL) {
                inp[i] = '\0';
                argv[(*argc)++] = start;
                start = NULL;
            }
            while (inp[i + 1] == ' ')
                i++;
        }

        else if (start == NULL) {
            start = &inp[i];
        }
        // Обработка экранированных символов
        if (inp[i] == '\\' && type_quotes == '\"' && (inp[i + 1] == '\\' || inp[i + 1] == '\"')) {
          memmove(&inp[i], &inp[i + 1], strlen(&inp[i + 1]) + 1); // Удаляем обратный слэш
          continue;
        }
        else if (inp[i] == '\\' && inp[i + 1] == ' '){
          memmove(&inp[i], &inp[i + 1], strlen(&inp[i + 1]) + 1);
          continue;
        }
        else if (!in_quotes && inp[i] == '\\' && (inp[i + 1] == '\'' || inp[i + 1] == '\"' || inp[i + 1] == 'n')){
          memmove(&inp[i], &inp[i + 1], strlen(&inp[i + 1]) + 1);
          continue;
        }
    }

    if (start != NULL) {
        argv[(*argc)++] = start;
    }
    argv[*argc] = NULL;
}

void fork_func(char *full_path, char **argv, char *outf, short int err_f, short int app){
  pid_t pid = fork();
  if (pid == 0) {
    if (outf){
      int flags = O_WRONLY | O_CREAT | (app? O_APPEND : O_TRUNC);
      int fd = open(outf, flags, 0666);
      if (fd == -1){
        perror("open");
        exit(1);
      }
      if (err_f){
        dup2(fd, STDERR_FILENO);
      }
      else {
        dup2(fd, STDOUT_FILENO);
      }
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

int autocomp(char* w){
  for (int i = 0; i < 2; i++){
    if (strncmp(w, data_autocompleting[i], 3) == 0){
      strcpy(w, data_autocompleting[i]);
      return 1;
    }
  }
  return 0;
}

int main() {
  char input[100];
  while (1){
    char c;
    int i = 0;
    setbuf(stdout, NULL);
    printf("$ ");
    // Wait for user input
    //   fgets(input, 100, stdin);
    while ((c = getchar()) != '\n') {
      if (c == '\t') {
          char word[5] = {0};
          int k = 0;
          for (int j = i - 1; j >= 0 && input[j] != ' '; j--) {
              word[k++] = input[j];
          }
          word[k] = '\0';
          strrev(word);
          if (autocomp(word)) {
              // Обновляем строку ввода
              strcpy(&input[i - k], word);
              i = i - k + strlen(word); // Обновляем индекс
              printf("\r$ %s ", input); // Добавляем пробел после автодополнения
              fflush(stdout);
          }
      } else {
          input[i++] = c;
      }
    }
    input[i] = '\0'; // Завершаем строку ввода
   // input[strlen(input) - 1] = '\0';
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
      short int err_f = 0;
      short int appen = 0;
      parse_input(input, argv, &argc, &output_file, &err_f, &appen);
      char *pth = check_path(argv[0]); // возвращаю полный путь до команды например cat, а затем применяю эту команду к аргументам argv
      if (pth != NULL)
        fork_func(pth, argv, output_file, err_f, appen); 
      else {
        printf("%s: command not found\n", argv[0]); 
      }
    }
  }
  return 0;
}
