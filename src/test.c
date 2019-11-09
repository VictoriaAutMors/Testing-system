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
    struct dirent *entry;
    DIR * dir1 = opendir("tmp");
    char * right_way = NULL;
    while((entry = readdir(dir1)) != NULL){
        right_way = GetRightWay(entry->d_name);
        char * cmd[3] = {right_way, right_way, NULL};
        if(fork() == 0){
            if(execvp(cmd[0], cmd) < 0){
                err(1, NULL);
            }
            return EXIT_SUCCESS;
        }
        wait(NULL);
    }
    return 0;
}