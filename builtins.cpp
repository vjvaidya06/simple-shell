#include "builtins.hpp"
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <print>
#include <optional>
#include <array>

namespace mysh{
    void exit(int status_code){
        std::exit(status_code);
    }
    std::filesystem::path pwd(){
        std::error_code ec{};
        std::filesystem::path cwd = std::filesystem::current_path(ec);
        if (ec){
            std::println("Threw error: {}", ec.message());
        }
        return cwd;
    }

    std::error_code cd(const std::optional<std::filesystem::path> destination){
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
        return ec;
        
    }

    std::optional<std::filesystem::path> which(const std::filesystem::path executable){
        std::error_code ec{};
        if (executable.has_parent_path()){
            if (std::filesystem::exists(executable, ec)){
                if (ec){
                    std::println("which threw error: {}", ec.message());
                }
                return executable;
            }
            // return std::nullopt;
        }
        // char paths[3][15] = {"/usr/local/bin", "/usr/bin", "/bin"};
        // std::array paths{"/usr/local/bin", "/usr/bin", "/bin"};
        std::array<std::filesystem::path, 3> paths{"/usr/local/bin", "/usr/bin", "/bin"};
        for (const auto& path : paths){
            if (std::filesystem::exists(path / executable, ec)){
                if (ec){
                    std::println("which threw error: {}", ec.message());
                }
                return path / executable;
            }
        }
        return std::nullopt;
    }

    bool isBuiltin(const std::string command){
        if (command == "exit" || command == "pwd" || command == "cd" || command == "which"){
            return true;
        }
        return false;
    }
}
