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

int genc = 10;

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
    while (ctest = readdir("tests") != NULL) {
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
    if (!write(fd, "\n", 1) {
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
    dir_bin = open_dir("tmp");
    dir_tests = opentest();
    while((entry_bin = readdir(dir_bin)) != NULL){
        if((pid = fork()) == 0) {
            entry_tests = ctest(entry_bin -> d_name);
            if (entry_tests == NULL) {
                ch_dir("..");
                ch_dir("..");
                ch_dir("tmp");
                write_result("-");
                return EXIT_SUCCESS;
            }
            for (int i = 1; i <= genc; i++) {
                ch_dir(entry_bin -> d_name);
                sprintf(test_name, "03%d.dat", i);
                fd = open(test_name, O_RDONLY);
                if (fd < 0) {
                    err(1, NULL);
                }
                dup2(STDIN_FILENO, fd);
                if (fork() == 0) {
                    char *exec = prepare_exec(entry_bin -> d_name);
                    ssize_t fdd = open("anss", O_WRONLY | O_CREAT | O_TRUNC, 0755)
                    dup2(STDOUT_FILENO, fdd);
                    if (execlp(exec, exec, NULL) < 0) {
                        write_result(entry_bin - > d_name, "-");
                        close fdd;
                        return EXIT_FAILURE;
                    }
                    return EXIT_SUCCESS;
                }
                wait()
            directory_name[0] = entry_tmp->d_name[0];
            dir2 = opendir(directory_name);
            int n = count_dat(dir2);
            char * test_name = malloc(8 * sizeof(char));
            if(n == 0){
                return EXIT_SUCCESS;
            }
                if(fork() == 0){
                    if(execvp(cmd[0], cmd) < 0){
                        err(1, NULL);
                    }
                    return EXIT_SUCCESS;
                }
                
                /*char ch;
                char * buf = NULL;
                int j = 0;
                while(read(1, &ch, 1)){
                    buf = (char *)realloc(buf, (j + 1) * sizeof(char));
                    buf[j] = ch;
                    j++;
                }
                sprintf(test_name, "00%d.ans", i);
                while ((entry_code = readdir(dir2)) != NULL)
                {
                    if(strcmp(entry_code->d_name, test_name) == 0){
                        strcpy(filename_out, getenv("PWD"));
                        strcat(filename_out, "/");
                        strcat(filename_out, directory_name);
                        strcat(filename_out, "/");
                        strcat(filename_out, test_name);
                        fd_out = open(filename_out, O_RDONLY, 0644);
                    }
                }
                char * test = NULL;
                j = 0;
                while(read(fd_out, &ch, 1)){
                    test = (char *)(realloc(test, (j + 1) * sizeof(char)));
                    test[j] = ch;
                    j++;
                }

                if(strcmp(buf, test) == 0){
                    printf("+");   
                }
                else{
                    printf("-");
                }
                close(fd_out);*/
                close(fd_in);
                
            }
                chdir(strcat(getenv("PWD"), "/../.."));
                closedir(dir2);
                return EXIT_SUCCESS;
        }
        wait(NULL);
    }
    closedir(dir1);
    return 0;
}