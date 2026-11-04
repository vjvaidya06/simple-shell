#include <print>
#include <ranges>
#include "tokenizer.hpp"

/*
We want to keep tokenizing till we find an "end" character. Then we execute.
With the power of canonical mode we just go line by line
Do I want to support a multiline command with quotes?

If I do, we need to encounter an end character, and the state needs to be None

End characters:
\n at the end of a token
;
|
\0 in the old shell, but probably not here

Also need to consider wildcards
If something contains a wildcard, expand it and skip forward
Ignore end characters?
In bash semicolons are still considered valid end characters within a wildcard expansion
given *.txt;ls, *.txt will be expanded
 */
namespace{
    inline bool is_end_char(char c){
        return c == ';' || c =='\n' || c == '|' || c == '\0';
    }
}

namespace mysh{
    //Use the default
    // Tokenizer::Tokenizer(){
    //     state = TokenizerStates::None;
    //     std::vector<std::string> tokens{};
    //     std::size_t end_pos{0};
    //     std::vector<const char*> command{};
    //     //Make the assumption that end characters are always at the end of a token
    //     //Might need a pair here to keep track of the end char
    //     // std::queue<std::size_t> ends{};
    // }

    //End character parsing
    //When an end character is found
    //TODO: backslash escaping
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
            // auto imp_tokens = word 
            auto imp_tokens = token 
                | std::views::enumerate
                | std::views::filter([](const auto& pair){
                        // return std::get<1>(pair) == '"' || std::get<1>(pair) == ';';
                        return std::get<1>(pair) == '"' || is_end_char(std::get<1>(pair));
                        });
            std::size_t last = 0;
            for (auto&& [index, character]: imp_tokens){
                // std::println("{}, {}", character, index);
                if (character == '"'){
                    tokens.back() += token.substr(last, index-last);
                    state = (state == TokenizerStates::None) ? TokenizerStates::Quote : TokenizerStates::None;
                }
                // else if (character == ';'){
                else{
                    //substr is really (start, len) and not (start, end)
                    //Damn you Bjarne
                    tokens.back() += token.substr(last, index-last);
                    //TODO: else push_back(nullptr)
                    if (state == TokenizerStates::Quote){
                        // tokens.back() += ';';
                        tokens.back() += character;
                    }
                    //Non commented end character is the only character in the word
                    //In this case tokens.back() is an empty string 
                    //Add the end character to that empty string instead of pushing back a new token
                    else if (token.size() == 1){
                        // tokens.pop_back();
                        tokens.back() += character;
                    }
                    else{
                        tokens.push_back(std::string(1, character));
                    }
                    //Do we need this?
                    if (index != word.size() - 1){
                        tokens.push_back("");
                    }
                    // if (state == TokenizerStates::None){
                        // tokens.push_back(nullptr);
                        //Ah damn it
                        //Use an empty string in place of nullptr
                        //Or just note the token in my queue?
                        // tokens.push_back("");
                        // ends.emplace(tokens.size());
                    // }
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
    //If there's an end character, create a char**
    //I also want to call erase after, how should that be managed?
    //I don't want fork/exec here
    //Option 1: decouple get command and erase
    //I'll have to call both. Just wrap it in a function in main
    //Option 2: let main handle it by passing things back
    //I don't want to do that one
    //THIS CANNOT OUTLIVE THE CLASS
    //AS SOON AS YOU CALL ERASE IT BECOMES A DANGLING POINTER
    //Must call erase after each call to this
    //I must also return the end character

    //Try something different.
    //Walk through until you find an end character.
    //If one can't be found:
    //  1. Keep track of where we stopped
    //  2. Wipe the vector
    std::optional<std::vector<const char*>> Tokenizer::getCommand(){
        if (tokens.empty()){
            return std::nullopt;
        }
        //TODO: Do we need this?
        // if (end_pos != 0){
        //     end_pos++;
        // }
        for (std::size_t i = end_pos; i < tokens.size(); i++){
            //Note that the command vector does not own the string
            command.push_back(tokens[i].c_str());
            if (tokens[i].size() == 1 && is_end_char(tokens[i][0])){
                command.push_back(nullptr);
                // return command;
                // I have to do this, or RVO breaks
                // Thank you Bjarne
                end_pos = i;
                return std::optional<std::vector<const char*>>{command};
            }
        }
        return std::nullopt;
    }

    void Tokenizer::eraseCommand(){
        if (tokens[end_pos].size() != 1 || !is_end_char(tokens[end_pos][0])){
            std::println("Called erase without a full command");
            return;
        }
        //If I forget this, command will be left with dangling pointers
        command.clear();
        tokens.erase(tokens.begin(), tokens.begin() + end_pos + 1);
        end_pos = 0;
        // if (ends.empty()){
        //     return;
        // }
        // tokens.erase(tokens.begin(), tokens.begin() + ends.front()+1);
        // ends.pop();
    }
}
