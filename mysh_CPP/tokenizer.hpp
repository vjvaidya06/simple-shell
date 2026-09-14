#ifndef MYSH_TOKENIZER
#define MYSH_TOKENIZER

#include <vector>
#include <string>
#include <array>
#include <queue>

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
 */
namespace mysh {
    enum class TokenizerStates{
        Comment,
        Quote,
        None
    };
    class Tokenizer{
        private:
            TokenizerStates state;
            //Should the class own the vector?
            std::vector<std::string> tokens;
            //[token, position in token]
            // std::optional<std::array<int, 2>> end;
            std::queue<std::array<int, 2>> end;

        public:
            Tokenizer();
            //Store end tokens if found
            //What if there's multiple commands in a single line?
            //Maybe call execute early
            //In that case maybe I want the execute here?
            //I don't want pure POSIX functions here
            //Then maybe this is a case where I want to declare the shell class as a friend
            //Other option is to just a queue for end
            //I can keep adding to it
            //Probably the better idea
            //Then I don't even need optional
            void tokenize(std::string line);
            
            //If end isn't nullopt
            //Erase the first end[0]-1 elements
            //Change tokens[end[0]] to substr(end[1]+1)
            void clearCommand();
            void clearTokens() {
                tokens.clear();
            }
            const std::queue<std::array<int, 2>>& getEnd(){
                return end;
            };
            const std::vector<std::string>& getTokens(){
                return tokens;
            };
    };
}


#endif
