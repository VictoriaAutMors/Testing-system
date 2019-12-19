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

/* functions that work with log files */

/* write error, date and time in log file */
void write_in_log(char *file, char *error) {
    FILE *fl = fopen(file, "a+");
    time_t rawtime;
    struct tm *timeinfo;
    if (time(&rawtime) < 0) {
        err(1, "get time error");
    }
    timeinfo = localtime(&rawtime);
    fprintf(fl, "%s %s\n", asctime(timeinfo), error);
    fclose(fl);
}

/* write register all activity in "file"*/
void logger(char *string, char *file, char *old_dir) {
    ch_dir("../logs");
    write_in_log(file, string);
    ch_dir("..");
    ch_dir(old_dir);
}

/* functions that work with FILEs and file descriptors*/

ssize_t fd_open(char *name) {
    ssize_t fd = open(name, O_RDWR | O_CREAT, 0755);
    if (fd < 0) {
        err(ERR, "failed to open file");
    }
    return fd;
}

/* closes n's file descriptors with va_list */
void fd_close(int fd1, ...) {
    va_list fd;
    ssize_t i;
    va_start(fd, fd1);
    for (i = fd1; i > 0; i = va_arg(fd, ssize_t)) {
        close(i);
    }
    va_end(fd);
}

FILE *fl_rfopen(char *name) {
    FILE *fl = fopen(name, "r");
    if (fl == NULL) {
        err(ERR, name, NULL);
    }
    return fl;
}

/* functions that obtain information from config files */

/* Read line from file. it returns the whole line */
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

/* Get number from line that ends with ';'. It returns that number*/
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

/* gets type of checking */
int get_check_type(char *line) {
    int i = 0, ans = 0;
    char error[1024];
    while(line[i] != ';') {
        i++;
    }
    if (line[i - 1] == 'i') {
        ans = INT;
    } else if (line[i - 1] == 'b') {
        ans = CHAR;
    } else {
        sprintf(error, "no information about check type");
        logger(error, "test_logs.txt", "contest");
        exit(1);
    }
    return ans;
}

/* functions open "file" and search line with "name". 
Flag defines what information is needed*/
int get_info(char *file, char *name, int flag) {
    FILE *fl = fopen(file, "r");
    char *line, *tmp = NULL, error[1024];
    int ans;
    if (fl == NULL) {
        sprintf(error, "failed to open %s ", file);
        logger(error, "test_logs.txt", "contest");
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
    logger("failed to get information from config files", "test_logs.txt",
                                                                 "contest");
    fclose(fl);
    exit(1);
}

/* functions that compare 2 FILEs */

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

/* it checks return status of program. After that calls checkers*/
int check_files(int status, FILE *fl1, FILE *fl2, char *task_name) {
    int check;
    ch_dir("../contest/");
    check = get_info("checker.cfg", task_name, CHECK_TYPE);
    ch_dir("../tmp");
    if (WEXITSTATUS(status) != 0) {
        return 1;
    }
    if(check == CHAR){
        return cmp_byte(fl1, fl2);
    }
    return cmp_int(fl1, fl2);
}

/* additional checking fucntions */

/* checks all compiled tasks' name with tasks' name in contest */
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

/* prepare argument vector and sets enviroment variable for exec */
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

/* Timer. If program works more than 5 seconds, it will be suspended*/
void handler(void) {
    putchar('-');
    logger("program running for too long", "test_logs.txt", "contest");
    kill(child, SIGKILL);
}

/* check all tests for task in loop */
int check_all_tasks(int count, char *test_name, DIR *dir, int time) {
    FILE *fl_ans, *fl_tmp;
    ssize_t fd_data, fd_tmp;
    int flag, status;
    char data_name[8], ans_name[8], error[1024];
    for (int i = 1; i <= count; i++) {
        flag = 0;
        ch_dir("tests");
        ch_dir(test_name);
        sprintf(data_name, "%03d.dat", i);
        sprintf(ans_name, "%03d.ans", i);
        fd_data = fd_open(data_name);
        fl_ans = fl_rfopen(ans_name);
        ch_dir("../../../tmp");
        fd_tmp = open("answer", O_RDWR | O_TRUNC | O_CREAT, 0755);
        alarm(time);
        if ((child = fork()) == 0) {
            char *exec = prepare_exec(test_name);
            dup2(fd_data, STDIN_FILENO);
            dup2(fd_tmp, STDOUT_FILENO);
            if (execl(exec, exec, NULL) < 0) {
                sprintf(error, "%s- failed on task %d", test_name, i);
                logger(error, "test_logs.txt", "tmp");
                fd_close(fd_tmp, fd_data);
                closedir(dir);
                exit(ERR);
            }
            fd_close(fd_tmp, fd_data);
            closedir(dir);
            return EXIT_SUCCESS;
        }
        waitpid(child, &status, 0);
        alarm(0);
        fd_close(fd_data);
        fl_tmp = fl_rfopen("answer");
        flag = check_files(status, fl_ans, fl_tmp, test_name);
        if (flag == 0) {
            putchar('+');
        } else {
            putchar('-');
            sprintf(error, "%s:Wrong answer on test %d", test_name, i);
            logger(error, "test_logs.txt", "tmp");
        }
        fclose(fl_tmp);
        fclose(fl_ans);
        ch_dir("../contest/");
    }
    putchar('\n');
    return EXIT_SUCCESS;
}

int test(void) {
    struct dirent *entry_bin;
    DIR *dir_bin;
    int time, test_count = 0;
    char error[1024];
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
                logger(error, "test_logs.txt", "contest");
            } else {
            test_count = get_info("global.cfg", entry_bin -> d_name, COUNT);
            time = get_info("global.cfg", "task_time", COUNT);
            check_all_tasks(test_count, entry_bin -> d_name, dir_bin, time);
            }
        }
    }
    closedir(dir_bin);
    return EXIT_SUCCESS;
}

int main(void) {
    ssize_t fd;
    signal(SIGALRM, (void (*)(int))handler);
    ch_dir("../logs");
    fd = open("Warnings&Errors.txt", O_WRONLY | O_CREAT, 0755);
    dup2(fd, STDERR_FILENO);
    test();
    fd_close(fd);
    return EXIT_SUCCESS;
}