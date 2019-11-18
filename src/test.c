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

DIR *opentest(void)
{
    ch_dir("contest");
    return open_dir("tests");
}

struct dirent *ctest(char *name)
{
    struct dirent *ctest;
    DIR * tests_dir;
    tests_dir = opendir("tests");
    while (ctest = readdir(tests_dir) != NULL) {
        if (strcmp(ctest -> d_name, name) == 0) {
            return ctest;
        }
    }
    return NULL;
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
    ch_dir("tmp");
    return exec;
}

int main(void)
{
    struct dirent *entry_bin, *entry_tests, *entry_test;
    DIR *dir_bin, *dir_tests;
    pid_t pid;
    ssize_t fd;
    char test_name[8];
    ch_dir("..");
    char ans_name[8];
    dir_bin = open_dir("tmp");
    dir_tests = opentest();
    while((entry_bin = readdir(dir_bin)) != NULL){
        if((pid = fork()) == 0) {
            entry_tests = ctest(entry_bin -> d_name);
            if (entry_tests == NULL) {
                ch_dir("..");
                ch_dir("..");
                ch_dir("tmp");
                write_result(entry_bin->d_name, "-");
                return EXIT_SUCCESS;
            }
            int flag = 0;
            for (int i = 1; i <= genc; i++) {
                ch_dir(entry_bin -> d_name);
                sprintf(test_name, "03%d.dat", i);
                fd = open(test_name, O_RDONLY);
                if (fd < 0) {
                    err(1, NULL);
                }
                dup2(STDIN_FILENO, fd);
                char *exec = prepare_exec(entry_bin -> d_name);
                ssize_t fdd = open("anss", O_WRONLY | O_CREAT | O_TRUNC, 0755);
                dup2(STDOUT_FILENO, fdd);

                if (fork() == 0) {
                    if (execlp(exec, exec, NULL) < 0) {
                        write_result(entry_bin -> d_name, "-");
                        close(fdd);
                        return EXIT_FAILURE;
                    }
                }
                wait(NULL);
                char * buf_answer = malloc(sizeof(char));  //array of program's answer
                char * buf_test = malloc(sizeof(char));    //array of test's content
                char ch = EOF;
                int j = 0;

                while(read(fdd, &ch, 1)){
                    buf_answer[j] = ch;
                    j++;
                    buf_answer = realloc(buf_answer, sizeof(char) * (j + 1));
                }
                close(fdd);
                ch_dir("..");
                ch_dir("contest");
                ch_dir("tests");
                ch_dir(entry_bin->d_name);
                sprintf(ans_name, "03%d.ans", i);
                int fddd = open(ans_name, O_RDONLY, 0755);
                j = 0;
                while (read(fddd, &ch, 1))
                {
                    buf_test[j] = ch;
                    j++;
                    buf_test = realloc(buf_test, sizeof(char) * 1); 
                }
                close(fddd);
                if(strstr(buf_test, buf_answer) != 0){
                    flag = 1;
                    break;
                }
                return EXIT_SUCCESS;
            }
                if(flag){
                    write_result(entry_bin->d_name, "-");
                }
                else{
                    write_result(entry_bin->d_name, "+");
                }
 
                return EXIT_SUCCESS;
        }
        wait(NULL);
    }
    return 0;
}