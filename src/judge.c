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

/* functions that work with path */

char *path_malloc(void)
{
    char *path = (char *)malloc(PATH_MAX * sizeof(char));
    if (path == NULL) {
        err(1, NULL);
    }
    return path;
}

char *get_exec_path(char *path, char *dname)
{
    char *exec_path = path_malloc();
    int len = strlen(dname);
    strcpy(exec_path, path);
    strcat(exec_path, "/../tmp/");
    strncat(exec_path, dname, len - 2);
    return exec_path;
}

char *get_cmpl_path(char *path, char *wdir, char *dname)
{
    char *cmpl_path = path_malloc();
    strcpy(cmpl_path, path);
    strcat(cmpl_path, "/");
    strcat(cmpl_path, wdir);
    strcat(cmpl_path, "/");
    strcat(cmpl_path, dname);
    return cmpl_path;
}

/* functions that work with dirctories */

void change_dir(char *name)
{
    char dir[PATH_MAX];
    if (getcwd(dir, PATH_MAX) == NULL) {
        err(1, NULL);
    }
    if (dir == NULL) {
        err(1, NULL);
    }
    strcat(dir, name);
    if (chdir(dir) < 0) {
        err(1, name, NULL);
    }
}

DIR *open_dir(char *name)
{
    DIR *dir = opendir(name);
    if (dir == NULL) {
        err(1, name, NULL);
    }
    return dir;
}

void get_cwd(char *wd)
{
    if (!getcwd(wd, PATH_MAX)) {
        err(1, NULL);
    }
}

/* other */

void compile(char *cmpl_path, char *exec_path)
{
    char *cmd[8] = {"gcc", cmpl_path, "-o", exec_path, "-Wall", "-Werror", "-lm", NULL};
    if (execvp(cmd[0], cmd) < 0) {
        err(1, NULL);
    }
}

int main(void){
    pid_t pid;
    DIR *pdir, *cdir;
    struct dirent  *pcat, *ccat;
    char *cmpl_path = NULL, *exec_path = NULL;
    char code_dir[PATH_MAX], src_dir[PATH_MAX];
    get_cwd(src_dir);
    change_dir("/../contest");
    pdir = open_dir("code");
    change_dir("/code");
    get_cwd(code_dir);
    while ((pcat = readdir(pdir)) != NULL) {
        cdir = open_dir(pcat -> d_name);
        while ((ccat = readdir(cdir)) != NULL) {
            if (strstr(ccat -> d_name, ".c") != NULL) {
                cmpl_path = get_cmpl_path(code_dir, pcat -> d_name, ccat -> d_name);
                exec_path = get_exec_path(src_dir, ccat -> d_name);
                if ((pid = fork()) == 0) {
                    compile(cmpl_path, exec_path);
                    return EXIT_SUCCESS;
                }
                wait(NULL);
                free(cmpl_path);
                free(exec_path);
            }
        }
        closedir(cdir);
    }
    closedir(pdir);
    return 0;
}