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

char *get_prg_name(char *name, char *dname)
{
    int len = strlen(dname);
    name = (char *)realloc(name, len * sizeof(char));
    if (name == NULL) {
        err(1, NULL);
    }
    strncpy(name, dname, len - 2);
    name[len - 1] = '\0';
    return name;
}

char *get_exec_path(char *exec_path, char *dname)
{
    int len = strlen(dname);
    exec_path = (char *)malloc((len + 10) * sizeof(char));
    if (exec_path == NULL) {
        err(1, NULL);
    }
    strcpy(exec_path, "../users/");
    strcat(exec_path, dname);
    return exec_path;
}

int main(void){
    pid_t pid;
    char *name  = NULL, *exec_path = NULL;
    struct dirent  *entry;
    DIR* dir1 = opendir("../users");
    if (dir1 == NULL) {
        err(1, NULL);
    }
    while ((entry = readdir(dir1)) != NULL) {
        if (strstr(entry -> d_name, ".c") != NULL) {
            name = get_prg_name(name, entry -> d_name);
            exec_path = get_exec_path(exec_path, entry -> d_name);
            if ((pid = fork()) == 0) {
                if (fork() == 0) {
                    char * cmd[7] = {"gcc", exec_path, "-o", name, "-Wall", "-Werror", NULL};
                    if (execvp(cmd[0], cmd) < 0) {
                        err(1, NULL);
                    }
                    free(exec_path);
                    free(name);
                    return EXIT_SUCCESS;
                }
                wait(NULL);
                /*if (execl(name_exec_prg, name_exec_prg, NULL) < 0) {
                    err(1, NULL);
        `    }*/
                free(exec_path);
                free(name);
                return EXIT_SUCCESS;
            }
            free(exec_path);
            free(name);   
        }
    }
    closedir(dir1);
    printf("\nIm sure\n");
    return 0;
}