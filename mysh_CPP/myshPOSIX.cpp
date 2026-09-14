#include <print>
#include <unistd.h>
#include "tokenizer.hpp"
#include "myshPOSIX.hpp"
#include "builtins.hpp"

namespace mysh{
    void posixMain(){
        Tokenizer tokenizer{};
        //Multiple "s or ;s in one token breaks it
        std::string line = "ls -alh  > \"test space.txt\" # this is a comment\n ls; \"hmm\" ";
        tokenizer.tokenize(line);
        auto tokens = tokenizer.getTokens();
        std::println("tokens");
        for (const auto& token: tokenizer.getTokens()){
            std::println("{}", token);
        }
    }
}
