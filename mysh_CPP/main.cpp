#include <print>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include "builtins.hpp"
#include "myshPOSIX.hpp"

int main (int argc, char **argv) {
    // if (argc > 1){
        // std::println("{} -> {}", argv[1], mysh::which(argv[1]).value_or("Not found"));
        // std::println("{} -> {}", argv[1], mysh::which(argv[1]).transform([](const auto& p){return p.string();}).value_or("Not found"));
    // }
    bool interactive = false;
    if (isatty(1) && argc == 1){
        std::println("Welcome to my shell");
        interactive = true;
    }
    else{
        std::println("Batch mode");
    }
    mysh::posixMain();
    return 0;
}
