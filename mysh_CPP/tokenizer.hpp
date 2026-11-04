#ifndef MYSH_TOKENIZER
#define MYSH_TOKENIZER

#include <vector>
#include <string>
#include <optional>
#include <queue>

namespace mysh {
    enum class TokenizerStates{
        Comment,
        Quote,
        None
    };
    enum class EndTokens{
        Stdout,
        Pipe
    };
    class Tokenizer{
        private:
            TokenizerStates state {TokenizerStates::None};
            //IMPORTANT: Can't push a nullptr into a vector of std::string
            //Can we use legacy C strings?
            //No, I'm relying on string concatenation too much, and the I want the vector to own them
            //Instead, we'll generate the final char**, or rather vec<char*> in the call command step
            //Should empty strings represent nullptr? Since I can't push nullptr directly
            //I could always just do std::vector<std::optional<std::string>>
            std::vector<std::string> tokens{};
            // std::vector<char*> tokens;
            //[token, position in token]
            // std::optional<std::array<int, 2>> end;
            std::size_t end_pos{0};
            std::vector<const char*> command{};

        public:
            Tokenizer() = default;
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

            std::vector<std::string> expandWildcard(std::string pattern);
            
            //If end isn't nullopt
            //Erase the first end[0]-1 elements
            //Change tokens[end[0]] to substr(end[1]+1)
            void clearCommand();
            void clearTokens() {
                tokens.clear();
            }
            const std::vector<std::string>& getTokens(){
                return tokens;
            };
            std::optional<std::vector<const char*>> getCommand();
            void eraseCommand();
    };
}


#endif
