# Mysh C Version
This is the original C Version of mysh.

## Quickstart

```bash
make
./mysh
```

## Features

### Built ins
which, cd, pwd and exit are built in shell commands

### External programs
External programs like ls are accessed by searching the "/usr/local/bin", "/usr/bin" and "/bin/" in that order. Once a program is found the shell forks a child process and then uses exec to replace it with the external program.

### Redirection
Just like bash, mysh supports both file redirection and piping. When redirecting, it uses dup2 to replace the stdin or stdout file descriptor with that of a file or a pipe.
