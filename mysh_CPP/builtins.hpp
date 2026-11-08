#ifndef _MYSH_BUILTINS_HPP
#define _MYSH_BUILTINS_HPP

#include <filesystem>
#include <expected>
#include <system_error>


namespace mysh{
    void exit(int status_code);
    // std::filesystem::path pwd();
    [[nodiscard]]
    std::expected<std::filesystem::path, std::error_code> pwd();
    // std::error_code cd(const std::optional<std::filesystem::path> destination);
    [[nodiscard]]
    std::optional<std::error_code> cd(const std::optional<std::filesystem::path> destination);
    // std::optional<std::filesystem::path> which(const std::filesystem::path path);
    [[nodiscard]]
    std::expected<std::filesystem::path, std::string> which(const std::filesystem::path executable);
    // bool isBuiltin(const std::string command);
    [[nodiscard]]
    bool isBuiltin(std::string_view command);
}

#endif
