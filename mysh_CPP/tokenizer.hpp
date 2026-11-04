#ifndef _MYSH_TOKENIZER
#define _MYSH_TOKENIZER

#include <variant>
#include <vector>
#include <string>
#include <optional>
#include <filesystem>

namespace mysh {
    enum class TokenizerStates{
        Comment,
        Quote,
        None
    };
    struct NoRedirect{};
    struct PipeRedirect{};
    struct FileRedirect{
        std::filesystem::path file;
    };
    using RedirectionState = std::variant<NoRedirect, PipeRedirect, FileRedirect>;
    class Tokenizer{
        private:
            TokenizerStates state {TokenizerStates::None};
            std::vector<std::string> tokens{};
            std::size_t end_pos{0};
            std::vector<const char*> command{};
            RedirectionState stdinRedirect = NoRedirect{};
            RedirectionState stdoutRedirect = NoRedirect{};

        public:
            Tokenizer() = default;
            void tokenize(std::string line);

            const std::vector<std::string>& getTokens(){
                return tokens;
            };
            std::optional<std::vector<const char*>> getCommand();
            void eraseCommand();
    };
}


#endif
