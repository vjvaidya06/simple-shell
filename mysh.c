#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include "commands.h"
#include "wildcard.h"


char *buf = NULL;
char **args = NULL;
int asize = 10;

//Anything returned by wildcard expansion is not part of the original buffer and therefore must be freed separately.
//This array will keep track of those pointers
char **freearr = NULL;
int fsize = 0;
int child = 0;

void cleanup(){
    if (isatty(STDIN_FILENO) && child == 0){
        write(STDOUT_FILENO, "Goodbye\n", 8);
    }
    if (buf != NULL){
        free(buf);
    }
    if (args != NULL){
        free(args);
    }
    if (freearr != NULL){
        for (int i = 0; i < fsize; i++){
            free(freearr[i]);
        }
        free(freearr);
    }
}

//Adds to the args array, handles realloc
void addArg(int a, char *w){
    if (a < asize){
        args[a] = w;
    }else{
        char** temp = realloc(args, asize*2*sizeof(char*));
        if (temp == NULL){
            perror("realloc fail\n");
            exit(1);
        }
        args = temp;
        asize*=2;
        args[a] = w;
    }
}

//Wildcard expansion function
void wildcards(int *a){
    if (containsWildcard(args[*a-1])){
        int sz = 0;
        char **exp = expand_wildcard(args[*a-1],&sz);
        //If there is at least one expansion, replace the original argument with the *
        if (sz > 0){
            --*a;
            for (int l = 0; l < sz; l++){
                addArg(*a, exp[l]);
                ++*a;
            }
            if (fsize == 0){
                freearr = exp;
            }else{
                char **temp = realloc(freearr, sizeof(char*)*(fsize+sz));
                if (temp == NULL){
                    perror("realloc fail\n");
                    exit(1);
                }
                freearr = temp;
                for (int l = 0; l < sz; l++){
                    freearr[fsize+l] = exp[l];
                }
                free(exp);
            }
            fsize += sz;
        }else{
            free(exp[0]);
            free(exp);
        }
    }
}

int main(int argc, char** argv){
    atexit(cleanup);
    //Ignore control c, only exit with exit
    signal(SIGINT, SIG_IGN);
    char* home = getenv("HOME");
    //printf("home: %s\n", home);
    if (isatty(STDIN_FILENO) && argc == 1){
        write(STDOUT_FILENO, "Welcome to my shell\n", 20);
    }
    if (argc > 1){
        //Batch mode with an arg
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0){
            perror("open failed\n");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    args = malloc(10*sizeof(char*));
    asize = 10;
    if (args == NULL){
        perror("malloc fail");
        exit(1);
    }
    long arg_max = sysconf(_SC_ARG_MAX);
    buf = malloc(arg_max+1);
    if (buf == NULL){
        perror("malloc fail\n");
        exit(1);
    }
    ssize_t n = 1;
    int a = 0;
    if (args == NULL){
        perror("malloc fail\n");
        exit(1);
    }
    char* in = NULL;
    char* out = NULL;
    int pipefd[2];
    //Checks if we came from a pipe
    //Also stores previous fd
    int pipebefore = -1;
    //Checks if this line ends with a pipe
    int pipeafter = 0;
    //If we're going to a pipe, switch STDOUT to the write end.
    //If we came from a pipe, switch STDIN to the read end
    while ((isatty(STDIN_FILENO) && argc == 1) || n != 0){
        if (isatty(STDIN_FILENO) && argc == 1){
            char* wd = pwd();
            write(STDOUT_FILENO," \n", 2);
            if (strncmp(wd, home, strlen(home)) == 0){
                wd[strlen(home)-1] = '~';
                write(STDOUT_FILENO,wd+strlen(home)-1, strlen(wd)-strlen(home)+1);
            }else{
                write(STDOUT_FILENO,wd, strlen(wd));
            }
            write(STDOUT_FILENO,"$ ", 2);
            free(wd);
        }
        n = read(STDIN_FILENO, buf, arg_max);
        in = NULL;
        out = NULL;
        //For safety
        buf[n] = '\0';
        int i = 0;
        while (i < n + 1){
            if (buf[i] == '#'){
                while (buf[i] != '\n' && buf[i] != '\0' && i < n + 1){
                    i++;
                }
            }
            //Check for special characters 
            if (buf[i] == '>'){
                i++;
                while (buf[i] == ' '){
                    i++;
                }
                int j = i;
                while (buf[i] != ' ' && buf[i] != '\n' && i < n + 1){
                    i++;
                }
                buf[i] = '\0';
                out = buf+j;
            }
            else if(buf[i] == '<'){
                i++;
                while (buf[i] == ' '){
                    i++;
                }
                int j = i;
                while (buf[i] != ' ' && buf[i] != '\n' && i < n + 1){
                    i++;
                }
                buf[i] = '\0';
                in = buf+j;
            }
            else{
                if (buf[i] != ' ' && buf[i] != '\0' && buf[i] != '\n' && buf[i] != '|'){
                    addArg(a, buf+i);
                    a++;
                }
                while (buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\0' && buf[i] != '|' && i < n + 1){
                    i++;
                }
                if (buf[i] == ' '){
                    buf[i] = '\0';
                    //check for and expand wildcard
                    if (a > 0){
                        wildcards(&a);
                    }
                }
                else if ((buf[i] == '\n' || buf[i] == '\0' || buf[i] == '|') && a > 0){
                    pipeafter = 0;
                    if (buf[i] == '|'){
                        pipeafter = 1;
                    }
                    buf[i] = '\0';
                    //check for and expand wildcards
                    wildcards(&a);
                    
                    addArg(a, NULL);
                    int aold = a;
                    a = 0;
                    //Built in commands
                    //Which and pwd handle | and >.
                    //Real Bash which doesn't support <, so mine won't either
                    if (strcmp("which", args[0]) == 0){
                        for (int j = 1; args[j] != NULL; j++){
                            char* path = which(args[j]);
                            int oldout = 0;
                            if (out != NULL){
                                oldout = dup(STDOUT_FILENO);
                                int fo = open(out, O_WRONLY | O_CREAT, 0640);
                                dup2(fo, STDOUT_FILENO);
                                close(fo);
                            }
                            else if (pipeafter == 1){
                                oldout = dup(STDOUT_FILENO);
                                pipe(pipefd);
                                dup2(pipefd[1], STDOUT_FILENO);
                                pipebefore = dup(pipefd[0]);
                            }
                            if (is_builtin(args[j])){
                                write(STDOUT_FILENO, args[j], strlen(args[j]));
                                write(STDOUT_FILENO, ": shell built-in command\n", 25);
                            }
                            else if (path == NULL){
                                write(STDOUT_FILENO, args[j], strlen(args[j]));
                                write(STDOUT_FILENO, " not found\n", 11);
                            }
                            else{
                                write(STDOUT_FILENO, path, strlen(path));
                                write(STDOUT_FILENO, "\n",1);
                            }
                            if (out != NULL){
                                dup2(oldout, STDOUT_FILENO);
                                close(oldout);
                                out = NULL;
                            }
                            else if (pipeafter == 1){
                                close(pipefd[0]);
                                close(pipefd[1]);
                                dup2(oldout, STDOUT_FILENO);
                                close(oldout);
                            }
                            if (path != NULL){
                                free(path);
                            }
                        }
                        in = NULL;
                        out = NULL;
                        
                    }
                    else if (strcmp("pwd", args[0]) == 0){
                        char* path = pwd();
                        if (path == NULL){
                            perror("pwd fail\n");
                            exit(1);
                        }
                        int oldout = 0;
                        if (out != NULL){
                            oldout = dup(STDOUT_FILENO);
                            int fo = open(out, O_WRONLY | O_CREAT, 0640);
                            dup2(fo, STDOUT_FILENO);
                            close(fo);
                        }
                        else if (pipeafter == 1){
                            oldout = dup(STDOUT_FILENO);
                            pipe(pipefd);
                            dup2(pipefd[1], STDOUT_FILENO);
                            pipebefore = dup(pipefd[0]);
                        }
                        write(STDOUT_FILENO, path, strlen(path));
                        write(STDOUT_FILENO, "\n", 1);
                        free(path);
                        if (out != NULL){
                            dup2(oldout, STDOUT_FILENO);
                            close(oldout);
                            out = NULL;
                        }
                        else if (pipeafter == 1){
                            close(pipefd[0]);
                            close(pipefd[1]);
                            dup2(oldout, STDOUT_FILENO);
                            close(oldout);
                        }
                        in = NULL;
                        out = NULL;
                        //printf("pwd\n");
                    }
                    else if (strcmp("cd", args[0]) == 0){
                        //CD uses the last arg
                        if (aold == 1){
                            cd(NULL, 1);
                        }else{
                            cd(args[aold-1], 2);
                        }
                    }
                    else if (strcmp("exit", args[0]) == 0){
                        exit(0);
                    }
                    //Regular programs
                    else{
                        char* path = which(args[0]);
                        if (path == NULL){
                            write(STDOUT_FILENO, args[0], strlen(args[0]));
                            write(STDOUT_FILENO, ": command not found\n", 20);
                        }
                        else{
                            if (pipeafter == 1){
                                pipe(pipefd);
                            }
                            pid_t pid = fork();
                            if (pid == 0){
                                child = 1;
                                signal(SIGINT, SIG_DFL);
                                if (!isatty(STDIN_FILENO) || argc > 1){
                                    int dn = open("/dev/null", O_RDONLY);
                                    dup2(dn, STDIN_FILENO);
                                    close(dn);
                                }
                                if (in != NULL){
                                    if (access(in, F_OK) != 0){
                                        write(STDOUT_FILENO, "file not found\n", 15);
                                        exit(0);
                                    }
                                    int fi = open(in, O_RDONLY);
                                    dup2(fi, STDIN_FILENO);
                                    close(fi);
                                }
                                if (out != NULL){
                                    int fo = open(out, O_WRONLY | O_CREAT, 0640);
                                    dup2(fo, STDOUT_FILENO);
                                    close(fo);
                                }
                                in = NULL;
                                out = NULL;
                                //If there's stuff in the pipe we need to fetch for the current command
                                if (pipebefore != -1){
                                    dup2(pipebefore, STDIN_FILENO);
                                    close(pipebefore);
                                }
                                if (pipeafter == 1){
                                    dup2(pipefd[1], STDOUT_FILENO);
                                    //Tell the next command it needs to read from the pipe
                                    pipebefore = 1;

                                }
                                if (pipeafter == 1){
                                    close(pipefd[0]);
                                    close(pipefd[1]);
                                }
                                execv(path, args);

                                perror("exec fail\n");
                                exit(1);
                            }
                            else{
                                int status = 0;
                                wait(&status);
                                if (status != 0){
                                    //Fine, just this once
                                    printf("Exited with status %d\n", status);
                                    fflush(stdout);
                                }
                                //If nonzero status, print
                                if (WIFSIGNALED(status)){
                                    psignal(WTERMSIG(status), "Terminated by signal ");
                                }
                                in = NULL;
                                out = NULL;
                                if (pipebefore != -1){
                                    close(pipebefore);
                                }
                                if (pipeafter == 1){
                                    //Hold onto the read end, we'll need it later.
                                    pipebefore = dup(pipefd[0]);
                                    close(pipefd[0]);
                                    close(pipefd[1]);
                                }else{
                                    pipebefore = -1;
                                }
                            }
                            in = NULL;
                            out = NULL;
                            free(path);
                        }
                    }
                }
            }
            i++;
        }
    }
    return 0;
}
