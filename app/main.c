#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>

// Функция для разбиения входной строки на аргументы
void parse_input(char *inp, char **argv, int *argc, char **outf, short int *err_f) {
    char *start = inp;
    short int in_quotes = 0;
    char type_quotes = 0;
    *outf = NULL; // Инициализируем outf как NULL
    *err_f = 0;   // Инициализируем err_f как 0

    for (int i = 0; inp[i]; i++) {
        // Обработка перенаправления вывода
        if (inp[i] == '1' && inp[i + 1] == '>' && !in_quotes) {
            inp[i] = '\0';
            *outf = &inp[i + 2];
            while (**outf == ' ') (*outf)++;
            break;
        } else if (inp[i] == '>' && !in_quotes) {
            inp[i] = '\0';
            *outf = &inp[i + 1];
            while (**outf == ' ') (*outf)++;
            break;
        } else if (inp[i] == '2' && inp[i + 1] == '>' && !in_quotes) {
            inp[i] = '\0';
            *outf = &inp[i + 2];
            while (**outf == ' ') (*outf)++;
            *err_f = 1;
            break;
        }

        // Обработка кавычек
        if ((inp[i] == '\'' || inp[i] == '\"') && !in_quotes) {
            in_quotes = 1;
            type_quotes = inp[i];
            start = &inp[i + 1];
        } else if (inp[i] == type_quotes && in_quotes) {
            in_quotes = 0;
            inp[i] = '\0';
            argv[(*argc)++] = start;
            start = NULL;
        }

        // Обработка пробелов
        if (inp[i] == ' ' && !in_quotes) {
            if (start != NULL) {
                inp[i] = '\0';
                argv[(*argc)++] = start;
                start = NULL;
            }
            while (inp[i + 1] == ' ') i++;
        } else if (start == NULL) {
            start = &inp[i];
        }
    }

    // Добавляем последний аргумент
    if (start != NULL) {
        argv[(*argc)++] = start;
    }
    argv[*argc] = NULL; // Завершаем массив аргументов NULL
}

// Функция для выполнения команды с перенаправлением вывода
void fork_func(char *full_path, char **argv, char *outf, short int err_f) {
    pid_t pid = fork();
    if (pid == 0) {
        if (outf) {
            int flags = O_WRONLY | O_CREAT | O_TRUNC;
            int fd = open(outf, flags, 0666);
            if (fd == -1) {
                perror("open");
                exit(1);
            }
            if (err_f) {
                dup2(fd, STDERR_FILENO); // Перенаправляем stderr
            } else {
                dup2(fd, STDOUT_FILENO); // Перенаправляем stdout
            }
            close(fd);
        }
        execv(full_path, argv);
        perror("execv"); // Если execv завершился с ошибкой
        exit(1);
    } else if (pid < 0) {
        perror("fork");
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

// Функция для поиска полного пути к команде
char *check_path(char *f) {
    char *path_check = getenv("PATH");
    if (path_check == NULL) return NULL;

    char *path_copy = strdup(path_check);
    char *dir = strtok(path_copy, ":");
    static char full_path[1024];

    while (dir != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, f);
        if (access(full_path, F_OK) == 0) {
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
    while (1) {
        printf("$ ");
        fflush(stdout);

        // Ждем ввода пользователя
        if (!fgets(input, sizeof(input), stdin)) {
            break; // Если ввод завершен (Ctrl+D)
        }
        input[strlen(input) - 1] = '\0'; // Убираем символ новой строки

        // Обработка команды exit
        if (strcmp(input, "exit 0") == 0) {
            exit(0);
        }

        // Обработка команды type
        else if (strncmp(input, "type ", 5) == 0) {
            if (strcmp(input, "type type") == 0) {
                printf("type is a shell builtin\n");
            } else if (strcmp(input, "type exit") == 0) {
                printf("exit is a shell builtin\n");
            } else if (strcmp(input, "type echo") == 0) {
                printf("echo is a shell builtin\n");
            } else if (strcmp(input, "type pwd") == 0) {
                printf("pwd is a shell builtin\n");
            } else if (strcmp(input, "type cd") == 0) {
                printf("cd is a shell builtin\n");
            } else {
                char *path = check_path(&input[5]);
                if (path) {
                    printf("%s is %s\n", &input[5], path);
                } else {
                    printf("%s: not found\n", &input[5]);
                }
            }
        }

        // Обработка команды pwd
        else if (strcmp(input, "pwd") == 0) {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("getcwd");
            }
        }

        // Обработка команды cd
        else if (strncmp(input, "cd ", 3) == 0) {
            char *dir = &input[3];
            if (strcmp(dir, "~") == 0) {
                dir = getenv("HOME");
            }
            if (chdir(dir) == -1) {
                printf("cd: %s: No such file or directory\n", dir);
            }
        }

        // Обработка других команд
        else {
            char *argv[10];
            int argc = 0;
            char *output_file = NULL;
            short int err_f = 0;
            parse_input(input, argv, &argc, &output_file, &err_f);

            if (argc > 0) {
                char *path = check_path(argv[0]);
                if (path) {
                    fork_func(path, argv, output_file, err_f);
                } else {
                    printf("%s: command not found\n", argv[0]);
                }
            }
        }
    }
    return 0;
}