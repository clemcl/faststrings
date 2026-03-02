#ifndef FAST_STRING_HPP
#define FAST_STRING_HPP

#include <cstdint>
#include <cstring>
#include <stdexcept>

template <std::size_t N>
class fast_string
{
    static_assert(N > 1, "Size must be greater than 1");

    char        data_[N];
    std::uint32_t len_;

public:
    constexpr std::size_t capacity() const noexcept { return N - 1; }
    std::uint32_t size() const noexcept { return len_; }
    bool empty() const noexcept { return len_ == 0; }

    fast_string() noexcept : len_(0)
    {
        data_[0] = '\0';
    }

    fast_string(const char* s)
    {
        assign(s);
    }

    void clear() noexcept
    {
        len_ = 0;
        data_[0] = '\0';
    }

    void assign(const char* s)
    {
        std::size_t slen = std::strlen(s);

        if (slen > capacity())
            slen = capacity();

        std::memcpy(data_, s, slen);
        data_[slen] = '\0';
        len_ = static_cast<std::uint32_t>(slen);
    }

    void append(const char* s)
    {
        std::size_t slen = std::strlen(s);
        std::size_t space = capacity() - len_;

        if (slen > space)
            slen = space;

        std::memcpy(data_ + len_, s, slen);
        len_ += static_cast<std::uint32_t>(slen);
        data_[len_] = '\0';
    }

    void append(const fast_string& other)
    {
        std::size_t space = capacity() - len_;
        std::size_t add = other.len_;

        if (add > space)
            add = space;

        std::memcpy(data_ + len_, other.data_, add);
        len_ += static_cast<std::uint32_t>(add);
        data_[len_] = '\0';
    }

    int compare(const fast_string& other) const noexcept
    {
        std::size_t min = len_ < other.len_ ? len_ : other.len_;

        int r = std::memcmp(data_, other.data_, min);
        if (r != 0)
            return r;

        if (len_ < other.len_) return -1;
        if (len_ > other.len_) return 1;
        return 0;
    }

    const char* c_str() const noexcept { return data_; }
    char* data() noexcept { return data_; }

    // For VB file integration
    std::uint32_t length() const noexcept { return len_; }
    void set_length(std::uint32_t l) noexcept
    {
        if (l <= capacity())
        {
            len_ = l;
            data_[len_] = '\0';
        }
    }
};

#endif