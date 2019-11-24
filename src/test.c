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
        err(1, dir, NULL);
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
            closedir(tests_dir);
            return TRUE;
        }
    }
    closedir(tests_dir);
    return FALSE;
}

void write_result(char *name, char *result)
{
    ssize_t fd = open("results.txt", O_WRONLY | O_CREAT | O_TRUNC, 0755);
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
    char path[4096];
    char *exec = (char *)malloc(4 * sizeof(char));
    strcpy(exec, name);
    ch_dir("..");
    ch_dir("..");
    ch_dir("..");
    ch_dir("tmp");
    if (getcwd(path, 4096) == NULL) {
        err(1, NULL);
    }
    setenv("PWD", path, 1);
    return exec;
}

ssize_t fd_ropen(char *name)
{
    ssize_t fd = open(name, O_RDONLY);
    if (fd < 0) {
        err(1, name, NULL);
    }
    return fd;
}

char *fill_from_fd(ssize_t fd)
{
    int i = 0;
    char ch, *buff = NULL, *tmp = NULL;
    while (read(fd, &ch, 1) > 0) {
        tmp = (char *)realloc(buff, (i + 1) * sizeof(char));
        if (tmp == NULL) {
            err(1, NULL);
        }
        buff = tmp;
        buff[i] = ch;
        i++;
        
    }
    return buff;
}

int cmp(ssize_t fd1, ssize_t fd2)
{
    char ch1, ch2;
    /*char buff1[256], buff2[256];
    int i = 0;
    char ch;
    while(read(fd1, &ch, 1) > 0){
        buff1[i] = ch;
        i++;
    }
    buff1[i] = '\0';
    i = 0;

    while (read(fd2, &ch, 1) > 0) {
        buff2[i] = ch;
        i++;
    }
    buff2[i] = '\0';
    printf("first: %s\n", buff1);
    printf("second: %d %s\n", i, buff2);
    if(strcmp(buff1, buff2) == 0){
        printf("equal\n");
        return 0;
    } else {
        printf("not equal\n");
        return 1;
    }*/
    
    while (read(fd1, &ch1, 1) > 0) {
        if (read(fd2, &ch2, 1) < 0) {
            return 1;
        }
        if (ch1 != ch2) {
            return 1;
        }
    }
    if (read(fd2, &ch2, 1) > 0) {
        return 1;
    }
    return 0;
}

int main(void)
{
    struct dirent *entry_bin;
    DIR *dir_bin;
    pid_t pid;
    ssize_t fd_data, fd_tmp, fd_ans;
    char test_name[8], ans_name[8];
    int i, flag = 0;
    ch_dir("..");
    dir_bin = open_dir("tmp");
    ch_dir("contest");
    while((entry_bin = readdir(dir_bin)) != NULL){
        if((pid = fork()) == 0) {
            if (is_test_legit(entry_bin -> d_name) == FALSE) {                
                write_result("%s -\n", entry_bin -> d_name);
                closedir(dir_bin);
                return EXIT_SUCCESS;
            }
            ch_dir("tests");
            for (i = 1; i <= genc; i++) {
                ch_dir(entry_bin -> d_name);

                flag = 0;
                sprintf(test_name, "%03d.dat", i);
                sprintf(ans_name, "%03d.ans", i);
                fd_data = fd_ropen(test_name);
                fd_ans = fd_ropen(ans_name);
                dup2(fd_data, STDIN_FILENO);
                fd_tmp = open("answer", O_RDWR | O_CREAT | O_TRUNC, 0755);
                if (fork() == 0) {
                    char *exec = prepare_exec(entry_bin -> d_name);
                    dup2(fd_tmp, STDOUT_FILENO);
                    //dup2(STDOUT_FILENO, fd_tmp);
                    if (execl(exec, exec, NULL) < 0) {
                        printf("%s %s\n", entry_bin -> d_name, "-");
                        close(fd_tmp);
                        return EXIT_FAILURE;
                    }
                    closedir(dir_bin);
                    return EXIT_SUCCESS;
                }
                wait(NULL);
                lseek(fd_tmp, 0, SEEK_SET);
                if (cmp(fd_ans, fd_tmp) == 1){
                    flag = 1;
                }

                ch_dir("..");
            }
            if (flag) {
                printf("%s %s\n", entry_bin -> d_name, "-");
                flag = 0;
            }
            else {
                printf("%s %s\n", entry_bin -> d_name, "+");
            }
            closedir(dir_bin);
            return EXIT_SUCCESS;
        }
        wait(NULL);
    }
    closedir(dir_bin);
    return 0;
}