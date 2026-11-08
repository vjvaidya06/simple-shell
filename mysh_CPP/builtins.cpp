#include "builtins.hpp"
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <system_error>
// #include <print>
#include <optional>
#include <array>

namespace mysh{
    void exit(int status_code){
        std::exit(status_code);
    }
    //TODO: Make these std::expected
    //std::filesystem::path pwd(){
    std::expected<std::filesystem::path, std::error_code> pwd(){
        using Result = std::expected<std::filesystem::path, std::error_code>;
        std::error_code ec{};
        std::filesystem::path cwd = std::filesystem::current_path(ec);
        if (ec){
            // std::println("Threw error: {}", ec.message());
            return Result(std::unexpect, ec);
        }
        return Result(cwd);
    }

    std::optional<std::error_code> cd(const std::optional<std::filesystem::path> destination){
        std::error_code ec{};
        if (!destination.has_value()){
            //No value, go to home
            std::filesystem::current_path(std::getenv("HOME"), ec);
        }
        else if (destination.value().is_relative()){
            std::filesystem::path home(std::getenv("HOME"));
            std::filesystem::current_path(home / *destination, ec);
        }
        //Absolute path
        else{
            std::filesystem::current_path(*destination, ec);
        }
        if (ec){
            return std::optional<std::error_code>(ec);
        }
        return std::nullopt;
        
    }

    // We'll have to rework this to make it cross platform later
    std::expected<std::filesystem::path, std::string> which(const std::filesystem::path executable){
        // using WhichError = std::expected<std::filesystem::path, std::error_code>;
        using WhichError = std::expected<std::filesystem::path, std::string>;
        std::error_code ec{};
        if (executable.has_parent_path()){
            if (std::filesystem::exists(executable, ec)){
                if (ec){
                    return WhichError(std::unexpect, ec.message());
                }
                return WhichError(executable);
            }
            return WhichError(std::unexpect, "Executable not found");
        }
        std::array<std::filesystem::path, 3> paths{"/usr/local/bin", "/usr/bin", "/bin"};
        for (const auto& path : paths){
            if (std::filesystem::exists(path / executable, ec)){
                if (ec){
                    return WhichError(std::unexpect, ec.message());
                }
                return WhichError(path / executable);
            }
        }
        return WhichError(std::unexpect, "Executable not found");
    }

    bool isBuiltin(std::string_view command){
        if (command == "exit" || command == "pwd" || command == "cd" || command == "which"){
            return true;
        }
        return false;
    }
}
