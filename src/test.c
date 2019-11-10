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

char * GetRightWay(char * name){
    char * right_way = (char *)malloc(4096 * sizeof(char));
    strcpy(right_way, getenv("PWD"));
    strcat(right_way, "/tmp/");
    strcat(right_way, name);
    return right_way;
}

int main(int argc, char ** argv){
    chdir(strcat(getenv("PWD"), "/.."));
    struct dirent *entry_tmp;
    struct dirent *entry_code;
    DIR * dir1 = opendir("tmp");
    DIR * dir2;
    char * right_way = NULL;
    char * directory_name = malloc(sizeof(char) * 2);
    directory_name[1] = '\0';
    int fd;
    char * filename = (char *)(malloc(sizeof(char) * 4096));
    while((entry_tmp = readdir(dir1)) != NULL){
        right_way = GetRightWay(entry_tmp->d_name);
        char * cmd[3] = {right_way, right_way, NULL};
        if(fork() == 0){
            chdir(strcat(getenv("PWD"), "/contest/tests"));
            directory_name[0] = entry_tmp->d_name[0];
            dir2 = opendir(directory_name);
            while ((entry_code = readdir(dir2)) != NULL)
            {
                if(strstr(entry_code->d_name, ".dat") != NULL){
                    strcpy(filename, getenv("PWD"));
                    strcat(filename, "/");
                    strcat(filename, directory_name);
                    strcat(filename, "/");
                    strcat(filename, entry_code->d_name);
                    fd = open(filename, O_RDONLY, 0644);
                    dup2(fd, 0);
                    break;
                }
            }
             
            chdir(strcat(getenv("PWD"), "/../.."));
            puts(cmd[0]);
            if(execvp(cmd[0], cmd) < 0){
                err(1, NULL);
            }
            return EXIT_SUCCESS;
            close(fd);

        }

        wait(NULL);
    }
    return 0;
}