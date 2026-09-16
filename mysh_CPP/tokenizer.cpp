#include <print>
#include <ranges>
#include <typeinfo>
#include "tokenizer.hpp"

namespace mysh{
    Tokenizer::Tokenizer(){
        state = mysh::TokenizerStates::None;
        std::vector<std::string> tokens{};
    }

    //For multiple quotes in one token
    //Sum them up, add 1 if we're in quote state
    //If it's odd, then stay in quote state
    //That part is simple
    //Now we get to the semicolons
    //If it's outside a quote, end the token early, mark it as an end
    //If not, just ignore it
    //a newline ends a comment. That's the rule I'll use.
    //We also don't support backslash escaping
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
            auto quotes_semicolons = word 
                | std::views::enumerate
                | std::views::filter([](const auto& pair){
                        return std::get<1>(pair) == '"' || std::get<1>(pair) == ';';
                        });
            std::size_t last = 0;
            for (auto&& [index, character]: quotes_semicolons){
                // std::println("{}, {}", character, index);
                if (character == ';'){
                    tokens.back() += token.substr(last, index-last);
                    if (state == TokenizerStates::Quote){
                        tokens.back() += ';';
                    }
                    //Non commented semicolon is the only character in the word
                    else if (token.size() == 1){
                        tokens.pop_back();
                    }
                    if (index != word.size() - 1){
                        tokens.push_back("");
                    }
                }
                else if (character == '"'){
                    tokens.back() += token.substr(last, index-last);
                    state = (state == TokenizerStates::None) ? TokenizerStates::Quote : TokenizerStates::None;
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
    }
}
