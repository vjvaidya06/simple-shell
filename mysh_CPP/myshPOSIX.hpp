#include <unistd.h>
#include <vector>
#include "tokenizer.hpp"

namespace mysh{
    void posixMain();
    class PosixShell{
        private:
            Tokenizer tokenizer;
            //Find an RAII way of managing this
            int pipe[2];
            bool pipe_active;

        public:
            ~PosixShell();
            //Read the input, call the tokenizer
            //Call command when encountering and end character
            void main();
            //
            void command(std::vector<std::string> tokens);
    };
}
