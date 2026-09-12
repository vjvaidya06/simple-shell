#ifndef SHELLBUILTINS
#define SHELLBUILTINS

#include <filesystem>


namespace mysh{
    void exit(int status_code);
    std::filesystem::path pwd();
    std::error_code cd(const std::optional<std::filesystem::path> destination);
    std::optional<std::filesystem::path> which(const std::filesystem::path path);
    bool isBuiltin(const std::string command);
}

#endif
