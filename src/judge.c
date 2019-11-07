#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include  <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv){
    char name[] = "~/semestr2/Testing-system/code/user1";
    struct dirent  * entry;
    name[strlen(name) - 1] = '\0';
    DIR* dir1 = opendir(name);

    if(dir1 == NULL){
        perror("No such directory");
        exit(1);
    }

    puts("HElloo\n");
    while ((entry = readdir(dir1)) != NULL) {

    puts("HElloo\n");
        if(entry->d_type == 8){
            if(fork() == 0){
                char * name_executed_program = malloc(sizeof(char) * strlen(entry->d_name));
                strcpy(name_executed_program, entry->d_name);
                name_executed_program[strlen(name_executed_program) - 1] = '\0';
                name_executed_program[strlen(name_executed_program) - 1] = '\0';
                char * Path = name;
                Path = realloc(Path, 256 * sizeof(char));
                strcat(Path, "/");
                strcat(Path, entry->d_name);
                char * cmd[7] = {"gcc", Path, "-o", name_executed_program, "-Wall", "-Werror", NULL};
                execvp(cmd[0], cmd);
                char exec_value[] = "./";
                strcat(exec_value, name_executed_program);
                char * cmd1[2] = {exec_value, NULL};
                execvp(cmd1[0], 
                       cmd1);
                free(name_executed_program);
            }
        }
    }
    closedir(dir1);
    return 0;
}