#include <stdio.h>
#include <fcntl.h>
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
#include <math.h>

#define TRUE 1
#define FALSE 0

int genc = 5;

char * GetRightWay(char * name){
    char * right_way = (char *)malloc(4096 * sizeof(char));
    strcpy(right_way, getenv("PWD"));
    strcat(right_way, "/tmp/");
    strcat(right_way, name);
    return right_way;
}

int count_dat(DIR * dir){
    struct dirent *entry;
    int i = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if(strstr(entry->d_name, ".dat") != NULL){
            i++;
        }
    }
    return i;
}

void ch_dir(char *dir)
{
    if (chdir(dir) < 0) {
        err(1, NULL);
    }
}

DIR *open_dir(char *dir)
{
    DIR *tmp = opendir(dir);
    if (tmp == NULL) {
        err(1, NULL);
    }
    return tmp;
}

int is_test_legit(char *name)
{
    struct dirent *ctest;
    DIR * tests_dir;
    tests_dir = open_dir("tests");
    while ((ctest = readdir(tests_dir)) != NULL) {
        if (strcmp(ctest -> d_name, name) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

void write_result(char *name, char *result)
{
    ssize_t fd = open("results.txt", O_WRONLY | O_CREAT, 0755);
    if (fd < 0) {
        err(1, NULL);
    }
    if (!write(fd, name, strlen(name))) {
        err(1, NULL);
    }
    if (!write(fd, result, strlen(result))) {
        err(1, NULL);
    }
    if (!write(fd, "\n", 1)){
        err(1, NULL);
    }
    close(fd);
}

char *prepare_exec(char *name)
{
    char *exec = (char *)malloc(4 * sizeof(char));
    strcpy(exec, "./");
    strcat(exec, name);
    ch_dir("..");
    ch_dir("..");
    ch_dir("..");
    ch_dir("tmp");
    return exec;
}

ssize_t fd_ropen(char *name)
{
    ssize_t fd = open(name, O_RDONLY);
    if (fd < 0) {
        err(1, NULL);
    }
    return fd;
}

char *fill_from_fd(ssize_t fd)
{
    int i = 0;
    char ch, *buff = NULL, *tmp = NULL;
    while (read(fd, &ch, 1)) {
        buff[i] = ch;
        i++;
        tmp = (char *)realloc(buff, (i + 1) * sizeof(char));
        if (tmp == NULL) {
            err(1, NULL);
        }
        buff = tmp;
    }
    return buff;
}

int main(void)
{
    struct dirent *entry_bin;
    DIR *dir_bin;
    pid_t pid;
    ssize_t fd_data, fd_tmp, fd_ans;
    char test_name[8], ans_name[8], *buff_answer, *buff_test;
    int i, flag = 0;
    ch_dir("..");
    dir_bin = open_dir("tmp");
    ch_dir("contest");
    while((entry_bin = readdir(dir_bin)) != NULL){
        if((pid = fork()) == 0) {
            if (is_test_legit(entry_bin -> d_name) == FALSE) {
                ch_dir("..");
                ch_dir("..");
                ch_dir("tmp");
                write_result(entry_bin->d_name, "-");
                return EXIT_SUCCESS;
            }
            for (i = 1; i <= genc; i++) {
                ch_dir(entry_bin -> d_name);
                sprintf(test_name, "03%d.dat", i);
                sprintf(ans_name, "03%d.ans", i);
                fd_data = fd_ropen(test_name);
                fd_ans = fd_ropen(ans_name);
                dup2(STDIN_FILENO, fd_data);
                fd_tmp = open("answer", O_WRONLY | O_CREAT | O_TRUNC, 0755);
                if (fork() == 0) {
                    char *exec = prepare_exec(entry_bin -> d_name);
                    dup2(STDOUT_FILENO, fd_tmp);
                    if (execlp(exec, exec, NULL) < 0) {
                        write_result(entry_bin -> d_name, "-");
                        close(fd_tmp);
                        return EXIT_FAILURE;
                    }
                    return EXIT_SUCCESS;
                }
                wait(NULL);
                buff_test = fill_from_fd(fd_tmp);
                close(fd_tmp);
                buff_answer = fill_from_fd(fd_ans);
                close(fd_ans);
                if(strstr(buff_test, buff_answer) != 0){
                    flag = 1;
                    break;
                }
            }
            if (flag) {
                write_result(entry_bin -> d_name, "-");
            }
            else {
                write_result(entry_bin -> d_name, "+");
            }
            return EXIT_SUCCESS;
        }
        wait(NULL);
    }
    return 0;
}