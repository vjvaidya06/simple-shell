CC = g++
CFLAGS = -std=c++23 -Wall -g

all: mysh

mysh: mysh.cpp builtins.cpp builtins.hpp
	$(CC) $(CFLAGS) -o mysh mysh.cpp builtins.cpp

