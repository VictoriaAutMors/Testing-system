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


enum return_value {
    FALSE,
    TRUE,
    ERR,
    COUNT,
    CHECK_TYPE,
    INT = -4,
    CHAR = -1
};


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
            read(fd, &ch, 1);
            count_of_tasks++;
            putchar(ch1);
            putchar(' ');
        }
        ch1 = ch;
    }
    putchar('\n');    
    return count_of_tasks;
}

int * how_many_test(int * count_of_tests){
    char ch, ch1, ch2;
    int j = 0;
    int fd = open("global.cfg", O_RDONLY);
    while(ch != '='){
        read(fd, &ch, 1);
    }
    while(read(fd, &ch, 1) > 0){
        if(ch1 == '='){
            count_of_tests[j] = atoi(&ch);
            j++;    
        }
        ch2 = ch1;
        ch1 = ch;
    }
    close(fd);
    return count_of_tests;
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
        exit(1);
    }
    return ans;
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


int get_info(char *file, char *name, int flag) {
    FILE *fl = fopen(file, "r");
    char *line, *tmp = NULL, error[1024];
    int ans;
    if (fl == NULL) {
        sprintf(error, "failed to open %s ", file);
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
    fclose(fl);
    exit(1);
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
        puts(pcat->d_name);
        
        if ((pid = fork()) == 0){ 
            int devNull = open("dev/null", O_WRONLY);
            dup2(devNull, 2);
            if (execl("./test", "./test", NULL) < 0) {
                err(1, "test");
            }
        }

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