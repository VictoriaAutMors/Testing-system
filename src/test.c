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

pid_t child;

enum return_value {
    FALSE,
    TRUE,
    ERR,
    COUNT,
    CHECK_TYPE,
    INT = -4,
    CHAR = -1
};

/* functions that work with directories */

void ch_dir(char *dir)
{
    if (chdir(dir) < 0) {
        err(1, dir, "change directory error");
    }
}

DIR *open_dir(char *dir)
{
    DIR *tmp = opendir(dir);
    if (tmp == NULL) {
        err(1, "open directory error");
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

void write_in_log(char *error) {
    FILE *fl = fopen("log.txt", "a");
    time_t rawtime;
    struct tm *timeinfo;
    if (time(&rawtime) < 0) {
        err(1, "get time error");
    }
    timeinfo = localtime(&rawtime);
    fprintf(fl, "%s %s\n", asctime(timeinfo), error);
    fclose(fl);
}

void logger(char *string, char *path) {
    ch_dir("../log");
    write_in_log(string);
    ch_dir("..");
    ch_dir(path);
}

char *prepare_exec(char *name)
{
    char path[PATH_MAX];
    char *exec = (char *)malloc(4 * sizeof(char));
    strcpy(exec, name);
    if (getcwd(path, PATH_MAX) == NULL) {
        err(1, "getcwd error");
    }
    setenv("PWD", path, TRUE);
    return exec;
}

ssize_t fd_open(char *name) {
    ssize_t fd = open(name, O_RDWR | O_CREAT, 0755);
    if (fd < 0) {
        err(ERR, "failed to open file");
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

FILE *fl_rfopen(char *name)
{
    FILE *fl = fopen(name, "r");
    if (fl == NULL) {
        err(1, name, NULL);
    }
    return fl;
}

int cmp_byte(FILE *fl1, FILE *fl2)
{
    char ch1, ch2;
    fseek(fl2, 0, SEEK_SET);
    while ((ch1 = fgetc(fl1)) > 0) {
        if ((ch2 = fgetc(fl2)) < 0) {
            return 1;
        }
        if (ch1 != ch2) {
            return 1;
        }
    }
    if ((ch2 = fgetc(fl2) != EOF)) {
        return 1;
    }
    return 0;
}

int cmp_int(FILE *fl1, FILE *fl2){
    int num1, num2;
    fseek(fl2, 0, SEEK_SET);
    while (fscanf(fl1, "%d", &num1) > 0) {
        if (fscanf(fl2, "%d", &num2) < 0) {
            return 1;
        }
        if (num1 != num2){
            return 1;
        }
    }
    if(fscanf(fl2, "%d", &num2) > 0){
        return 1;
    }
    return 0;
}

char *read_line(FILE *fl) {
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

int get_num(char *line) {
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

int get_check_type(char *line) {
    int i = 0, ans = 0;
    char *error = NULL;
    while(line[i] != ';') {
        i++;
    }
    if (line[i - 1] == 'i') {
        ans = INT;
    } else if (line[i - 1] == 'b') {
        ans = CHAR;
    } else {
        sprintf(error, "no information about check type");
        logger(error, "contest");
        exit(1);
    }
    return ans;
}

int get_info(char *file, char *name, int flag) {
    FILE *fl = fopen(file, "r");
    char *line, *tmp = NULL;
    int ans;
    if (fl == NULL) {
        sprintf(tmp, "failed to open %s ", file);
        logger(tmp, "contest");
        exit(1);
    }
    while ((line = read_line(fl)) != NULL) {
        tmp = strstr(line, name);
        if (tmp) {
            if (flag == COUNT) {
                ans = get_num(tmp);
            } else {
                ans = get_check_type(tmp);
            }
            free(line);
            fclose(fl);
            return ans;
        }
        free(line);
    }
    logger("failed to get information from config files", "contest");
    fclose(fl);
    exit(1);
}

void handler(void) {
    putchar('-');
    logger("program running for too long", "contest");
    kill(child, SIGKILL);
}

int check_files(int status, FILE *fl1, FILE *fl2, char *task_name) {
    int check;
    ch_dir("../contest/");
    check = get_info("checker.cfg", task_name, CHECK_TYPE);
    ch_dir("tests");
    ch_dir(task_name);
    ch_dir("../../../tmp");
    if (WEXITSTATUS(status) != 0) {
        return 1;
    }
    if(check == CHAR){
        return cmp_byte(fl1, fl2);
    }
    return cmp_int(fl1, fl2);
}

int main(void) {
    struct dirent *entry_bin;
    DIR *dir_bin;
    int status, i, test_count = 0, flag = 0;
    FILE *fl_tmp, *fl_ans;
    ssize_t fd_tmp, fd_data;
    char test_name[8], ans_name[8], error[1024];
    signal(SIGALRM, (void (*)(int))handler);
    ch_dir("..");
    dir_bin = open_dir("tmp");
    ch_dir("contest");
    while((entry_bin = readdir(dir_bin)) != NULL) {
        if(!strstr(entry_bin -> d_name, ".") && strcmp(entry_bin -> d_name,
                                                    "answer")) {
            printf("%s ", entry_bin -> d_name);
            if (is_test_legit(entry_bin -> d_name) == FALSE) {
                putchar('-');
                sprintf(error, "%s- no test; ", entry_bin -> d_name);
                logger(error, "contest");
                return EXIT_SUCCESS;
            }
            test_count = get_info("global.cfg", entry_bin -> d_name, COUNT);
            for (i = 1; i <= test_count; i++) {
                flag = 0;
                ch_dir("tests");
                ch_dir(entry_bin -> d_name);
                sprintf(test_name, "%03d.dat", i);
                sprintf(ans_name, "%03d.ans", i);
                fd_data = fd_open(test_name);
                fl_ans = fl_rfopen(ans_name);
                ch_dir("../../../tmp");
                fd_tmp = open("answer", O_RDWR | O_TRUNC | O_CREAT, 0755);
                alarm(5);
                if ((child = fork()) == 0) {
                    char *exec = prepare_exec(entry_bin -> d_name);
                    dup2(fd_data, STDIN_FILENO);
                    dup2(fd_tmp, STDOUT_FILENO);
                    if (execl(exec, exec, NULL) < 0) {
                        putchar('-');
                        sprintf(error, "%s- failed on task %d", entry_bin -> d_name, i);
                        logger(error, "tmp");
                    }
                    close_fd(fd_tmp, fd_data);
                    closedir(dir_bin);
                    return EXIT_SUCCESS;
                }
                waitpid(child, &status, 0);
                alarm(0);
                close_fd(fd_tmp, fd_data);
                fl_tmp = fl_rfopen("answer");
                flag = check_files(status, fl_ans, fl_tmp, entry_bin -> d_name);
                if (flag == 0) {
                    putchar('+');
                } else {
                    putchar('-');
                    sprintf(error, "%s:Wrong answer on test %d", entry_bin->d_name, i);
                    logger(error, "tmp");
                }
                fclose(fl_ans);
                fclose(fl_tmp);
                ch_dir("../contest/");
            }
        }
        putchar('\n');
    }
    closedir(dir_bin);
    return EXIT_SUCCESS;
}