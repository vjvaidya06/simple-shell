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
                        //Don't add it, start searching it
                        tokens.emplace_back(token.substr(newline+1));
                    }
                    // std::println("Newline found");
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
                    tokens.back() += token.substr(last, index);
                    if (state == TokenizerStates::Quote){
                        tokens.back() += ';';
                    }
                    //What if a push_back an empty string here?
                    //This creates a situation where I might have an extra empty string
                    //When does this happen, and how to protect against it?
                    //This is all one token, so we'll keep emplacing back.
                    //Issue arises when ; is the last token, and we're outside a quote
                    //The next thing we do has to be emplace_back.
                    //In that case push back unless we're the last character
                    // std::println("index type: {}, word.size() type:{}", typeid(index).name(), typeid(word.size()).name());
                    if (index != word.size() - 1){
                        tokens.push_back("");
                    }
                }
                else if (character == '"'){
                    tokens.back() += token.substr(last, index);
                    state = (state == TokenizerStates::None) ? TokenizerStates::Quote : TokenizerStates::None;
                }
                last = index+1;
            }
            // std::println("Token: {}, last: {}, token.size(): {}", token, last, token.size());
            if (last < token.size()){
                if (tokens.size() == 0){
                    tokens.emplace_back(token.substr(last, token.size()));
                }
                else{
                    tokens.back() += token.substr(last, token.size());
                }
            }


            /*std::string_view token(word.begin(), word.end());
            std::size_t quote_pos = token.find('"');
            switch (state){
                case TokenizerStates::None:
                    if (word.empty()){
                        continue;
                    }
                    if (token[0] == '#'){
                        state = TokenizerStates::Comment;
                    }
                    else if (quote_pos == std::string::npos){
                        tokens.emplace_back(token);
                    }
                    else{
                        tokens.emplace_back(token.substr(0, quote_pos));
                        tokens.back() += token.substr(quote_pos+1);
                        state = TokenizerStates::Quote;
                    }
                    break;
                case TokenizerStates::Comment: {
                        std::size_t newline = token.find('\n');
                        if (newline != std::string::npos){
                            if (newline+1 < token.length()){
                                tokens.emplace_back(token.substr(newline+1));
                            }
                            // std::println("Newline found");
                            state = TokenizerStates::None;
                        }
                        break;
                    }
                case TokenizerStates::Quote: {
                        std::size_t quote = token.find('"');
                        if (quote != std::string::npos && quote > 0){
                            tokens.back() += " ";
                            tokens.back() += token.substr(0, quote);
                            state = TokenizerStates::None;
                        }
                    }
            }*/
        }
    }
}
