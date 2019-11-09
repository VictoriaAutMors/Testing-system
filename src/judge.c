#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
//#include <limits.h>
#include <sys/wait.h>
//#include <math.h>

char *get_prg_name(char *name, char *dname)
{
    int len = strlen(dname);
    if (name == NULL) {
        err(1, NULL);
    }
    strncpy(name, dname, len - 2);
    name[len - 1] = '\0';
    name[len - 2] = '\0';
    char * right_name = (char *)malloc(sizeof(char) * 4096);
    strcpy(right_name, getenv("PWD"));
    strcat(right_name, "/user1/");
    strcat(right_name, name);
    strcpy(name, right_name);
    free(right_name);
    return name;
}

char *get_exec_path(char *exec_path, char *dname)
{
    //int len = strlen(dname);
    if (exec_path == NULL) {
        err(1, NULL);
    }
    strcpy(exec_path, getenv("PWD"));
    strcat(exec_path, "/user1/");
    strcat(exec_path, dname);
    return exec_path;
}

int main(int argc, char ** argv){
    pid_t pid;
    char *name  = NULL, *exec_path = NULL;
    struct dirent  *entry;
    chdir(strcat(getenv("PWD"), "/../code"));
    DIR* dir1 = opendir("user1");
    if (dir1 == NULL) {
        err(1, NULL);
    }
    exec_path = (char *)malloc(sizeof(char) * 256);
    name = (char *)malloc(sizeof(char) * 4096);
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
                    return EXIT_SUCCESS;
                }
                wait(NULL);
                /*if (execl(name_exec_prg, name_exec_prg, NULL) < 0) {
                    err(1, NULL);
        `    }*/
                //free(exec_path);
                //free(name);
                return EXIT_SUCCESS;
            }
            wait(NULL);
            //free(name);   
        }
    }
    free(exec_path);
    free(name);
    closedir(dir1);
    return 0;
}