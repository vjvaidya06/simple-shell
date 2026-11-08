CC = g++
CFLAGS = -std=c++23 -Wall -g

all: mysh

mysh: main.cpp builtins.cpp builtins.hpp myshPOSIX.cpp myshPOSIX.hpp tokenizer.cpp tokenizer.hpp wildcardExpansion.cpp wildcardExpansion.hpp
	$(CC) $(CFLAGS) -o mysh main.cpp builtins.cpp myshPOSIX.cpp tokenizer.cpp wildcardExpansion.cpp

clean:
	rm -f mysh

