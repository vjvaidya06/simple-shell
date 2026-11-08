#include <cstdio>
#include <expected>
#include <print>
#include <ranges>
#include <variant>
#include "tokenizer.hpp"
#include "wildcardExpansion.hpp"

namespace{
    inline bool isEndChar(char c){
        return c == ';' || c =='\n' || c == '|' || c == '\0';
    }
}

namespace mysh{

    //File redirection handler
    //getCommand needs to call this
    bool Tokenizer::handleRedirect(std::size_t idx){
        if (idx < tokens.size() - 1){
            const auto& curtoken = tokens[idx];
            if (curtoken.size() == 1){
                if (curtoken[0] == '>' && std::holds_alternative<NoRedirect>(stdoutRedirect)){
                    stdoutRedirect = FileRedirect{std::filesystem::path(tokens[idx+1])};
                    return true;
                }
                else if (curtoken[0] == '<' && std::holds_alternative<NoRedirect>(stdinRedirect)){
                    stdinRedirect = FileRedirect{std::filesystem::path(tokens[idx+1])};
                    return true;
                }
            }
        }
        return false;
    }

    //An individual word can have more than one token
    //This handles that
    void Tokenizer::tokenizeWord(std::string_view token){
        auto imp_tokens = token 
        | std::views::enumerate
        | std::views::filter([](const auto& pair){
                // return std::get<1>(pair) == '"' || std::get<1>(pair) == ';';
                return std::get<1>(pair) == '"' || isEndChar(std::get<1>(pair));
                });
        std::size_t last = 0;
        for (auto&& [index, character]: imp_tokens){
            // std::println("{}, {}", character, index);
            if (character == '"'){
                tokens.back() += token.substr(last, index-last);
                state = (state == TokenizerStates::None) ? TokenizerStates::Quote : TokenizerStates::None;
            }
            else{
                //substr is really (start, len) and not (start, end)
                //Damn you Bjarne
                tokens.back() += token.substr(last, index-last);
                if (state == TokenizerStates::Quote){
                    // tokens.back() += ';';
                    tokens.back() += character;
                }
                //Non commented end character is the only character in the word
                //In this case tokens.back() is an empty string 
                //Destroy that extra empty string
                else{
                    if (token.size() == 1){
                        tokens.pop_back();
                    }
                    //Check if the second last token is < or >
                    //Expand wildcard, select the first term in the expansion
                    // std::size_t currentSize = tokens.size();
                    if (tokens.size() > 0){
                        expandWildcard(&tokens);
                    }
                    tokens.push_back(std::string(1, character));
                    //Note that we can only redirect to one thing at a time
                    //handleRedirect(currentSize);
                    //pipes overrule file redirects
                    /*if (character == '|'){
                        stdoutRedirect = PipeRedirect{};
                    }*/
                }
                //Do we need this?
                if (index != token.size() - 1){
                    tokens.push_back("");
                }
            }
            last = index+1;
        }
        if (last < token.size()){
            if (tokens.size() == 0){
                tokens.emplace_back(token.substr(last));
            }
            else{
                tokens.back() += token.substr(last);
            }
        }
    }

    //End character parsing
    //When an end character is found
    //TODO: backslash escaping
    //TODO: Redirection
    //TODO: How do we handle redirection state when we hold multiple commands in a line?
    //Let getCommand handle it instead
    void Tokenizer::tokenize(std::string line){
        auto result = line | std::views::split(' ');
        for (auto&& word : result){
            std::string_view token(word.begin(), word.end());
            if (state == TokenizerStates::None){
                if (word.empty()){
                    continue;
                }
                // std::println("Word: {}", token);
                if (token[0] == '#'){
                    state = TokenizerStates::Comment;
                }
                else{
                    tokens.push_back("");
                }
            }
            //Comment could end in the same token, so don't use else if
            if (state == TokenizerStates::Comment){
                std::size_t newline = token.find('\n');
                if (newline == std::string::npos){
                    continue;
                }
                else{
                    if (newline+1 < token.length()){
                        token.remove_prefix(newline+1);
                        tokens.push_back("");
                    }
                    state = TokenizerStates::None;
                }
            }
            else if (state == TokenizerStates::Quote){
                tokens.back() += " ";
            }
            //This needs to include all possible end characters
            tokenizeWord(token);
        }
    }

    std::optional<std::vector<const char*>> Tokenizer::getCommand(){
    // std::expected<std::vector<const char*>, CommandResult> Tokenizer::getCommand(){
        // using CommandError = std::expected<std::vector<const char*>, CommandResult>;
        if (tokens.empty()){
            return std::nullopt;
            // return CommandError(std::unexpect, CommandResult::Incomplete);
        }
        //TODO: Do we need this?
        // if (end_pos != 0){
        //     end_pos++;
        // }
        for (std::size_t i = end_pos; i < tokens.size(); i++){
            //Note that the command vector does not own the string
            //A trailing </> is treated as a literal < or >
            if (tokens[i].size() == 1 && isEndChar(tokens[i][0])){
                command.push_back(nullptr);
                /*if (i > 0){
                    if (tokens[i].size() == 1){
                        if (tokens[i][0] == '>' || tokens[i][0] == '<'){
                            return CommandError(std::unexpect, CommandResult::Malformed);
                        }
                    }
                }*/
                end_pos = i;
                // return command;
                // I have to do this, or RVO breaks
                // Thank you Bjarne
                return std::optional<std::vector<const char*>>{command};
                // return CommandError(command);
            }
            command.push_back(tokens[i].c_str());
        }
        return std::nullopt;
        // return CommandError(std::unexpect, CommandResult::Incomplete);
    }

    void Tokenizer::eraseCommand(){
        if (tokens[end_pos].size() != 1 || !isEndChar(tokens[end_pos][0])){
            std::println("Called erase without a full command");
            return;
        }
        //If I forget this, command will be left with dangling pointers
        command.clear();
        tokens.erase(tokens.begin(), tokens.begin() + end_pos + 1);
        end_pos = 0;
    }
}
