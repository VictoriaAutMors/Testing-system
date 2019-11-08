#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <limits.h>
#include <sys/wait.h>

int main(void){
    pid_t pid;
    struct dirent  *entry;
    char cwd[PATH_MAX], *path = NULL;
    if (getcwd(cwd, PATH_MAX) == NULL) {
        err(1, NULL);
    }
    DIR* dir1 = opendir(cwd);
    if (dir1 == NULL) {
        err(1, "No such directory");
    }
    while ((entry = readdir(dir1)) != NULL) {
        if (strstr(entry -> d_name, ".c") != NULL) {
            if ((pid = fork()) == 0) {
                char *name_exec_prg = (char *)malloc(sizeof(char) * (strlen(entry -> d_name) + 1));
                if (name_exec_prg == NULL) {
                    err(1, NULL);
                }
                strcpy(name_exec_prg, entry -> d_name);
                name_exec_prg[strlen(name_exec_prg) - 1] = '\0';
                name_exec_prg[strlen(name_exec_prg) - 1] = '\0';
                path = (char *)realloc(path, PATH_MAX * sizeof(char));
                if (path == NULL) {
                    err(1, NULL);
                }
                strcpy(path, cwd);
                strcat(path, "/");
                strcat(path, entry -> d_name);
                if (fork() == 0) {
                    char * cmd[7] = {"gcc", path, "-o", name_exec_prg, "-Wall", "-Werror", NULL};
                    if (execvp(cmd[0], cmd) < 0) {
                        err(1, NULL);
                    }
                    return EXIT_SUCCESS;
                }
                wait(NULL);
                if (execl(name_exec_prg, name_exec_prg, NULL) < 0) {
                    err(1, NULL);
                }
                free(path);
                free(name_exec_prg);
                return EXIT_SUCCESS;
            }
        }
    }
    closedir(dir1);
    return 0;
}