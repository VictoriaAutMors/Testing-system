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
#include <stdarg.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

pid_t child;

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

char *prepare_exec(char *name)
{
    char path[PATH_MAX];
    char *exec = (char *)malloc(4 * sizeof(char));
    strcpy(exec, name);
    if (getcwd(path, PATH_MAX) == NULL) {
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

void close_fd(int fd1, ...)
{
    va_list fd;
    ssize_t i;
    va_start(fd, fd1);
    for (i = fd1; i > 0; i = va_arg(fd, ssize_t)) {
        close(i);
    }
    va_end(fd);
}

int cmp_byte(ssize_t fd1, ssize_t fd2)
{
    char ch1, ch2;
    lseek(fd2, 0, SEEK_SET);
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

int cmp_int(ssize_t fd1, ssize_t fd2){
    int num1, num2;
    lseek(fd2, 0, SEEK_SET);
    dup2(0, fd2);
    while(scanf("%d", &num1) > 0){
        if(scanf("%d", &num2) < 0){
            return 1;
        }
        if(num1 != num2){
            return 1;
        }
    }
    if(scanf("%d", &num2) > 0){
        return 1;
    }
    return 0;
}

int write_result(int flag, char *name, int number)
{
    if (!flag) {
        printf("%s+; ", name);
    } else {
        printf("%s- failed on task %d; ", name, number);
        flag = 0;
    }
    return flag;
}

char *read_line(FILE *fl)
{
    int len = 0;
    char *line = (char *)malloc(LINE_MAX);
    if (line == NULL) {
        err(1, NULL);
    }
    if (fgets(line, LINE_MAX, fl) == NULL) {
        free(line);
        return NULL;
    }
    len = strlen(line);
    line[len - 1] = '\0';
    return line;
}

int get_num(char *line)
{
    int num = 0, i = 0;
    do {
        if (line[i] >= '0' && line[i] <= '9') {
            num *= 10;
            num += line[i] - '0';
        }
        i++;
    } while (line[i] != ';');
    return num;
}

int get_count(char *name)
{
    FILE *fl;
    char *line, *tmp;
    int num;
    if (strstr(name, ".")) {
        exit(0);
    }
    fl = fopen("global.cfg", "r");
    if (fl == NULL) {
        err(1, "failed to open global.cfg");
    }
    while ((line = read_line(fl)) != NULL) {
        tmp = strstr(line, name);
        if (tmp) {
            num = get_num(tmp);
            free(line);
            return num;
        }
        free(line);
    }
    printf("no informations about tests count for %s\n", name);
    exit(1);
}

void handler(void)
{
    kill(child, SIGKILL);
}

int how_to_check(){
    ssize_t fd = open("checker.cfg", O_RDONLY, 0644);
    char ch;
    char * buf = NULL; 
    int i = 0;
    while(read(fd, &ch, 1) > 0){
        buf = realloc(buf, (i + 1) * sizeof(char));
        buf[i] = ch;
        i++;
    }
    if(buf[10] == 'i'){
        free(buf);
        return 1;
    }
    if(buf[10] == 'b'){
        free(buf);
        return 0;
    }
    free(buf);
    return 2;
}

void logger(char * error){
    if(fork() == 0){
        ssize_t fd = open("logger.txt", O_WRONLY | O_CREAT, 0777);
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        dup2(0, fd);
        printf("[%d-%d-%d][%d:%d:%d]", 
                tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, 
                tm.tm_hour, tm.tm_min, tm.tm_sec); 
        puts(error);
        close(fd);
    }
}

int main(void)
{
    struct dirent *entry_bin;
    DIR *dir_bin;
    int status;
    ssize_t fd_data, fd_tmp, fd_ans;
    char test_name[8], ans_name[8];
    int i, test_count = 0, flag = 0;
    signal(SIGALRM, (void (*)(int))handler);
    ch_dir("..");
    dir_bin = open_dir("tmp");
    ch_dir("contest");
    while((entry_bin = readdir(dir_bin)) != NULL){
        if(strcmp(entry_bin -> d_name, "answer") && fork() == 0) {
            if (is_test_legit(entry_bin -> d_name) == FALSE) {
                char error[1024];
                putchar('-');
                sprintf(error, "%s- no test; ", entry_bin -> d_name);
                ch_dir("../logger");
            //    logger(error);
                ch_dir("../contest");
                closedir(dir_bin);
                return EXIT_SUCCESS;
            }
            test_count = get_count(entry_bin -> d_name);
            for (i = 1; i <= test_count; i++) {
                flag = 0;
                ch_dir("tests");
                ch_dir(entry_bin -> d_name);
                sprintf(test_name, "%03d.dat", i);
                sprintf(ans_name, "%03d.ans", i);
                fd_data = fd_ropen(test_name);
                fd_ans = fd_ropen(ans_name);
                dup2(fd_data, STDIN_FILENO);
                ch_dir("../../../tmp");
                fd_tmp = open("answer", O_RDWR | O_CREAT | O_TRUNC, 0755);
                if ((child = fork()) == 0) {
                    char *exec = prepare_exec(entry_bin -> d_name);
                    dup2(fd_tmp, STDOUT_FILENO);
                    alarm(5);
                    if (execl(exec, exec, NULL) < 0) {
                        putchar('-');
                        char error[1024]; 
                        sprintf(error, "%s- filed on task %d", entry_bin->d_name, i);
                        ch_dir("../logger");
             //           logger(error);
                        ch_dir("../tmp");
                        close_fd(fd_tmp);
                        return EXIT_FAILURE;
                    }
                    closedir(dir_bin);
                    return EXIT_SUCCESS;
                }
                waitpid(child, &status, 0);
                alarm(0);
                ch_dir("../contest/tests");
                ch_dir(entry_bin->d_name);
                int check = how_to_check();
                ch_dir("../../../tmp");
                if(check == 0){
                    if (WEXITSTATUS(status) != 0 || cmp_byte(fd_ans, fd_tmp)) {
                        close_fd(fd_tmp, fd_data, fd_ans);
                        flag = 1;
                        break;
                    }
                }
                if(check == 1){
                    if(WEXITSTATUS(status) != 0 || cmp_int(fd_ans, fd_tmp)){
                        close_fd(fd_tmp, fd_data, fd_ans);
                        flag = 1;
                        break;
                    }
                }
                if(check == 2){
                    putchar('-');
                    ch_dir("../logger");
                    //logger("No information about checker. please make 'checker.cfg' file in task's directory and choose the current type of checker.");
                    ch_dir("../tmp");
                    close_fd(fd_tmp, fd_data, fd_ans);
                    return 1;
                }
                if(flag == 0){
                    putchar('+');
                }
                if(flag == 1){
                    putchar('-');
                    char error[1024];
                    ch_dir("../logger");
                    sprintf(error, "%s:Wrong answer on test %d", entry_bin->d_name, i);

                    //logger(error);
                    ch_dir("../tmp");
                    close_fd(fd_tmp, fd_data, fd_ans);
                    return 1;
                }

                close_fd(fd_tmp, fd_data, fd_ans);
                ch_dir("../contest/");
            }
            //flag = write_result(flag, entry_bin -> d_name, i);
            closedir(dir_bin);
            putchar('\n');
            return EXIT_SUCCESS;
        }
        wait(NULL);
        putchar('\n');
    }
    closedir(dir_bin);
    return 0;
}