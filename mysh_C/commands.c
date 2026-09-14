#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

char* which(char* program){
    char* path = NULL;
    int fd = 0;
    if (program[0] == '/' || program[0] == '.'){
        if (access(program, F_OK) == 0){
            path = malloc(strlen(program)+1);
            if (path == NULL){
                perror("malloc fail\n");
                exit(1);
            }
            strcpy(path, program);
            return path;
        }
    }
    
    char paths[3][15] = {"/usr/local/bin", "/usr/bin", "/bin"};
    for (int i = 0; i < 3; i++){
        fd = open(paths[i], O_RDONLY | O_DIRECTORY);
        if (faccessat(fd, program, F_OK, 0) == 0){
            //program + path + / + \0
            path = malloc(strlen(program) + strlen(paths[i]) + 1 + 1);
            if (path == NULL){
                perror("malloc fail\n");
                exit(1);
            }
            snprintf(path, strlen(program)+strlen(paths[i])+2, "%s/%s", paths[i], program);
            close(fd);
            return path;
        }
        close(fd);
    }
    return NULL;
}

//argv should work as a single ptr instead of a double
int cd(char *argv, int argc) {
    const char *target;

    if (argc == 1) {
        // No argument: go to $HOME
        target = getenv("HOME");
        if (!target) {
            write(STDERR_FILENO, "cd: HOME not set\n", 17);
            return -1;
        }
    } else if (argc == 2) {
        target = argv;
    } else {
        write(STDERR_FILENO, "cd: too many arguments\n", 23);
        return -1;
    }

    if (chdir(target) != 0) {
        // chdir sets errno; print a useful message
        perror("cd");
        return -1;
    }

    return 0;
}

char* pwd() {
    char buf[4096];

    if (getcwd(buf, sizeof(buf)) == NULL) {
        perror("pwd");
        exit(1);
    }

    // Write path + newline (two writes avoids off-by-one if path fills buf)
    //size_t len = strlen(buf);
    //Don't forget the extra char for the \0
    //I want it returned as a string instead.
    
    //if (write(out_fd, buf, len) < 0 || write(out_fd, "\n", 1) < 0) {
        //perror("pwd: write");
        //return -1;
    //}

    return strdup(buf);
}

int is_builtin(const char *name) {
    return (strcmp(name, "cd")   == 0 ||
            strcmp(name, "pwd")  == 0 ||
            strcmp(name, "exit") == 0 ||
            strcmp(name, "which") == 0);
}
