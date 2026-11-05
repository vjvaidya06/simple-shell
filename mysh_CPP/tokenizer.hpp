#ifndef _MYSH_TOKENIZER
#define _MYSH_TOKENIZER

#include <variant>
#include <vector>
#include <string>
#include <expected>
#include <filesystem>

namespace mysh {
    enum class TokenizerStates{
        Comment,
        Quote,
        None
    };
    enum class CommandResult{
        Malformed,
        Incomplete
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

            void tokenizeWord(std::string_view token);
            bool handleRedirect(std::size_t currentSize);

        public:
            Tokenizer() = default;
            void tokenize(std::string line);

            const std::vector<std::string>& getTokens(){
                return tokens;
            };
            std::optional<std::vector<const char*>> getCommand();
            // std::expected<std::vector<const char*>, CommandResult> getCommand();
            void eraseCommand();
    };
}


#endif
