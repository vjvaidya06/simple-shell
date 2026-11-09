#ifndef POSIXFD_CLASS_HPP
#define POSIXFD_CLASS_HPP

#include <system_error>
#include <unistd.h>
#include <expected>
#include <filesystem>

namespace unistdHelper {
    class PosixFD{
        private:
            int m_fd;
            void destroy();

        public:
            explicit PosixFD(int fd) noexcept;
            explicit PosixFD() noexcept;
            // [[nodiscard]]
            // static std::expected<PosixFD, std::error_code> open(const std::filesystem::path& path, int flags);
            [[nodiscard]]
            static std::expected<PosixFD, std::error_code> open(const std::filesystem::path& path, int flags, mode_t mode=0666);
            ~PosixFD();
            //dup can error out, and should be explicit
            //better to just delete the copy functions
            //If you need a copy just make it yourself and use the constructor
            PosixFD(const PosixFD& other) = delete;
            PosixFD& operator= (const PosixFD& other) = delete;
            PosixFD(PosixFD&& other) noexcept;
            PosixFD& operator= (PosixFD&& other) noexcept;

            [[nodiscard]]
            explicit operator bool() const noexcept;

            [[nodiscard]]
            bool operator==(const PosixFD& other) const noexcept = default;

            [[nodiscard]]
            int release() noexcept;

            void reset(int fd=-1) noexcept;

            [[nodiscard]]
            int fd() const{return m_fd;};
    };
}

#endif
