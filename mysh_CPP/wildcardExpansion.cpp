#include <expected>
#include <print>
#include <filesystem>
#include <regex>
#include "wildcardExpansion.hpp"

namespace mysh{
    bool containsWildcard(std::string_view token){
        for (const auto& i: token){
            if (i  == '*'){
                return true;
            }
        }
        return false;
    }

    std::regex globtoRegex(std::string pattern){
        std::string regexstr = "";
        regexstr.reserve(pattern.size());
        for (char i : pattern){
            switch(i){
                case '*':
                    regexstr += ".*";
                    break;
                //I miss Rust
                case '.': case '^': case '$': case '+': case '?': case '(' : case ')' : case '[' : case ']' : case '{' : case '}' : case '|' : case '\\' : 
                    regexstr += '\\';
                    //Allow fallthrough
                default:
                    regexstr += i;
            }
        }
        // std::println("Matching pattern {}", regexstr);
        return std::regex(regexstr);
    }


    //Glob every token
    //Don't parse for special characters unless this returns nullopt
    //If it does, consider it literally
    // std::optional<std::vector<std::string>> expandWildcard(std::string_view pattern){
    std::vector<std::string> expandWildcard(std::string_view pattern){
        std::vector<std::string> matches{};
        std::filesystem::path path(pattern);
        std::regex regexstr = globtoRegex(path.filename());
        // std::println("Matching pattern {}", regexstr);
        auto directory = path.has_parent_path() ? path.parent_path() : std::filesystem::path(".");
        std::println("Parent directory: {}", directory.string());
        for (const auto& entry : std::filesystem::directory_iterator(directory)){
            if (entry.is_regular_file()){
                std::string filename = entry.path().filename().string();
                // std::println("Found filename {}", filename);
                if (std::regex_match(filename, regexstr)){
                    // std::println("Found match {}", filename);
                    matches.push_back(std::filesystem::path(directory / filename));
                }
            }
        }
        // for (const auto& path : matches){
        //     std::println("Path: {}", path);
        // }
        if (matches.size() == 0){
            matches.push_back(std::string(pattern));
        }
        // std::println("Name: {}", path.filename().string());
        // std::println("regex str: {}", regexstr);
        // std::println("{}", path.parent_path());

        return matches;
    }
    // bool expandWildcard(std::string_view pattern, std::vector<std::string> *tokens){
    bool expandWildcard(std::vector<std::string> *tokens){
        if (tokens == nullptr){
            return false;
        }
        if (!containsWildcard(tokens->back())){
            return true;
        }
        std::filesystem::path path(tokens->back());
        std::regex regexstr = globtoRegex(path.filename());
        // std::println("Matching pattern {}", regexstr);
        auto directory = path.has_parent_path() ? path.parent_path() : std::filesystem::path(".");
        // std::println("Parent directory: {}", directory.string());
        bool foundMatch = false;
        for (const auto& entry : std::filesystem::directory_iterator(directory)){
            if (entry.is_regular_file()){
                std::string filename = entry.path().filename().string();
                // std::println("Found filename {}", filename);
                if (std::regex_match(filename, regexstr)){
                    // std::println("Found match {}", filename);
                    if (!foundMatch){
                        tokens->pop_back();
                        foundMatch = true;
                    }
                    tokens->push_back(std::filesystem::path(directory / filename));
                }
            }
        }
        // for (const auto& path : matches){
        //     std::println("Path: {}", path);
        // }
        return true;
    }
}
