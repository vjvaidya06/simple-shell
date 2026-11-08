#include <system_error>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <cerrno>
#include <filesystem>
#include "PosixFD.hpp"

namespace unistdHelper {

    //Incorrect usage can result in invalid or double close
    //This is to the user's discretion, just like with unique_ptr
    //If close is set to false, the fd will not be closed 
    //Example usage would be when wrapping 0, 1, or 2
    PosixFD::PosixFD(int fd) noexcept : m_fd{fd}{}

    void PosixFD::destroy(){
        if (m_fd >= 0){
            close(m_fd);
        }
        m_fd = -1;
    }

    PosixFD::~PosixFD(){
        destroy();
    }
    
    PosixFD::PosixFD(PosixFD&& other) noexcept : m_fd{std::exchange(other.m_fd, -1)}{}

    PosixFD& PosixFD::operator= (PosixFD&& other) noexcept{
        if (this != &other){
            destroy();
            m_fd = std::exchange(other.m_fd, -1);
        }
        return *this;
    }

    PosixFD::operator bool() const noexcept{
        return m_fd > -1;
    }

    std::expected<PosixFD, std::error_code> PosixFD::open(const std::filesystem::path& path, int flags, mode_t mode){
        int fd = ::open(path.c_str(), flags, mode);
        if (fd == -1){
            return std::expected<PosixFD, std::error_code>(std::unexpect, std::error_code(errno, std::generic_category()));
        }
        return std::expected<PosixFD, std::error_code>(std::in_place, PosixFD(fd));
    }
    /*std::expected<PosixFD, std::error_code> PosixFD::open(const std::filesystem::path& path, int flags){
        int fd = ::open(path.c_str(), flags);
        if (fd == -1){
            return std::expected<PosixFD, std::error_code>(std::unexpect, std::error_code(errno, std::generic_category()));
        }
        return std::expected<PosixFD, std::error_code>(std::in_place, PosixFD(fd));
    }*/

    int PosixFD::release() noexcept{
        return std::exchange(m_fd, -1);
    }

    void PosixFD::reset(int fd) noexcept{
        destroy();
        m_fd = fd;
    }

}
