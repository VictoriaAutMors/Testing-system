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
#include <fcntl.h>

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

                  
void compile(char *cmpl_path, char *exec_path, char *name)
{
    char *cmd[9] = {"gcc", cmpl_path, "-o", exec_path, "-lm", NULL}; 
    if (execvp(cmd[0], cmd) < 0) {
        printf("%s - ", name);
    }
}

int print_header(){
    int fd;
    int count_of_tasks = 0;
    char ch, ch1;;
    int num;
    printf("                    ");
    fd = open("global.cfg", O_RDONLY, 0777);
    if(fd < 0){
        perror("file descriptor");
    }
    while(ch != '='){
        read(fd, &ch, 1);
    }
    while(read(fd, &ch, 1) > 0){
        if (ch == '='){
            count_of_tasks++;
            putchar(ch1);
            putchar(' ');
        }
        ch1 = ch;
    }
    putchar('\n');    
    return count_of_tasks;
}

int * how_much_test(int * count_of_tests){
    char ch, ch1;
    int j = 0;
    int fd = open("global.cfg", O_RDONLY, 0777);
    while(ch != '='){
        read(fd, &ch, 1);
    }
    while(read(fd, &ch, 1) > 0){
        if(ch1 == '='){
            count_of_tests[j] = atoi(&ch);
            j++;    
        }
        ch1 = ch;
    }
    close(fd);
    return count_of_tests;
}

int main(void){
    pid_t pid;
    DIR *pdir, *cdir;
    int count_of_tasks;
    struct dirent  *pcat, *ccat;
    char *cmpl_path = NULL, *exec_path = NULL;
    char code_dir[PATH_MAX], src_dir[PATH_MAX];
    get_cwd(src_dir);
    change_dir("/../contest");
    count_of_tasks = print_header(); //print_header and count quantity of tasks
    int *count_of_tests = (int *)malloc(sizeof(int) * count_of_tasks);//it's count of tests for each program
    printf("%d", count_of_tasks);
        
    count_of_tests = how_much_test(count_of_tests);
    
    printf("%d", count_of_tasks);

    for(int i = 0; i < count_of_tasks; i++){
        printf("%d", count_of_tests[i]);
    }
    pdir = open_dir("code");
    change_dir("/code");
    get_cwd(code_dir);
    int j = 0;
    while ((pcat = readdir(pdir)) != NULL && strstr(pcat -> d_name, ".") == NULL) {
        cdir = open_dir(pcat -> d_name);
        while ((ccat = readdir(cdir)) != NULL) {
            if (strstr(ccat -> d_name, ".c") != NULL) {
                cmpl_path = get_cmpl_path(code_dir, pcat -> d_name, ccat -> d_name);
                exec_path = get_exec_path(src_dir, ccat -> d_name);
                if ((pid = fork()) == 0) {
                    compile(cmpl_path, exec_path, ccat -> d_name);
                    return EXIT_SUCCESS;
                }
                wait(NULL);
                free(cmpl_path);
                free(exec_path);
            }
        }
        wait(NULL);
        change_dir("/../..");
        change_dir("/src");
        int fd[2];
        pipe(fd);

        printf("%s", pcat->d_name);
        int length = strlen(pcat->d_name);
        for(int i = 0; i < 20 - length; i++){
            printf(" ");
        }
        if ((pid = fork()) == 0) {
            dup2(fd[0], 1);
            int devNull = open("dev/null", O_WRONLY);
            dup2(devNull, 2);
            if (execl("./test", "./test", NULL) < 0) {
                err(1, "test");
            }
        }

        /* What I add*/
        int flag = 0;
        char ch;
        char * data_from_pipe = NULL;
        int l = 0;
        while(read(fd[1], &ch, 1) > 0){
            data_from_pipe = (char *)realloc(data_from_pipe, sizeof(char) * (l + 1));
            data_from_pipe[l] = ch;
            l++;
        }
        close(fd[0]);
        close(fd[1]);
        char genererating_name;
        for(int i = 0; i < count_of_tasks; i++){
            genererating_name = 'A' + i;
            int z = 0;
            int flag = 0;
            for(int k = 0; count_of_tests[j]; k++){
                if(data_from_pipe[k] == genererating_name){
                    z = k;
                    flag = 1;
                    break;
                }
            }
            if (flag == 0){
                printf("-");
            }
            else{
                for(int k = 0; k < count_of_tests[j]; k++){
                    if(data_from_pipe[z + k] == '-'){
                        printf("-");
                        break;
                    }   
                }
                printf("+");
            }
            printf(" ");
        }
        free(data_from_pipe);
        /* What I add*/
        wait(NULL);
        putchar('\n');
        change_dir("/..");
        change_dir("/contest/code");
        closedir(cdir);
        j++;
    }
    closedir(pdir);
    return 0;
}