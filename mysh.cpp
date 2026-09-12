#include <print>
#include <filesystem>
#include <cstdlib>
#include <csignal>
#include <optional>
#include <system_error>
#include "builtins.hpp"

int main (int argc, char **argv) {
    if (argc > 1){
        // std::println("{} -> {}", argv[1], mysh::which(argv[1]).value_or("Not found"));
        std::println("{} -> {}", argv[1], mysh::which(argv[1]).transform([](const auto& p){return p.string();}).value_or("Not found"));
    }
    return 0;
}
