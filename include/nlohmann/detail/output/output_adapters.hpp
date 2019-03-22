#pragma once

#include <algorithm> // copy
#include <cstddef> // size_t
#include <ios> // streamsize
#include <iterator> // back_inserter
#include <memory> // shared_ptr, make_shared
#include <ostream> // basic_ostream
#include <string> // basic_string
#include <vector> // vector

namespace nlohmann
{
namespace detail
{
/// abstract output adapter interface
template<typename CharType>
struct output_adapter_protocol
{
    /*!
    @brief write a character to the output
    @param[in] c  character to write
    */
    virtual void write_character(CharType c) = 0;

    /*!
    @brief write some characters to the output
    @param[in] s  pointer to a continouus chunk of memory
    @param[in] length  number of bytes to write
    */
    virtual void write_characters(const CharType* s, std::size_t length) = 0;

    virtual ~output_adapter_protocol() = default;
};

/// a type to simplify interfaces
template<typename CharType>
using output_adapter_t = std::shared_ptr<output_adapter_protocol<CharType>>;

/// output adapter for byte vectors
template<typename CharType>
class output_vector_adapter : public output_adapter_protocol<CharType>
{
  public:
    explicit output_vector_adapter(std::vector<CharType>& vec) noexcept
        : v(vec)
    {}

    void write_character(CharType c) override
    {
        v.push_back(c);
    }

    void write_characters(const CharType* s, std::size_t length) override
    {
        std::copy(s, s + length, std::back_inserter(v));
    }

  private:
    std::vector<CharType>& v;
};

/// output adapter for output streams
template<typename CharType>
class output_stream_adapter : public output_adapter_protocol<CharType>
{
  public:
    explicit output_stream_adapter(std::basic_ostream<CharType>& s) noexcept
        : stream(s)
    {}

    void write_character(CharType c) override
    {
        stream.put(c);
    }

    void write_characters(const CharType* s, std::size_t length) override
    {
        stream.write(s, static_cast<std::streamsize>(length));
    }

  private:
    std::basic_ostream<CharType>& stream;
};

/// output adapter for basic_string
template<typename CharType, typename StringType = std::basic_string<CharType>>
class output_string_adapter : public output_adapter_protocol<CharType>
{
  public:
    explicit output_string_adapter(StringType& s) noexcept
        : str(s)
    {}

    void write_character(CharType c) override
    {
        str.push_back(c);
    }

    void write_characters(const CharType* s, std::size_t length) override
    {
        str.append(s, length);
    }

  private:
    StringType& str;
};

template<typename CharType>
output_adapter_t<CharType> make_output_adapter(std::vector<CharType>& vec)
{
    return std::make_shared<output_vector_adapter<CharType>>(vec);
}

template<typename CharType>
output_adapter_t<CharType> make_output_adapter(std::basic_ostream<CharType>& s)
{
    return std::make_shared<output_stream_adapter<CharType>>(s);
}

template<typename CharType, typename StringType = std::basic_string<CharType>>
output_adapter_t<CharType> make_output_adapter(StringType& s)
{
    return std::make_shared<output_string_adapter<CharType, StringType>>(s);
}

template<typename CharType>
output_adapter_t<CharType> make_output_adapter(output_adapter_t<CharType> oa)
{
    return oa;
}

/*!
@brief Convenience wrapper around @ref output_adapter_protocol.
@tparam CharType  type of the characters (e.h., `char` or `std::uint8_t`)
@tparam StringType  string type derived from @a CharType
*/
template<typename CharType, typename StringType = std::basic_string<CharType>>
class output_adapter
{
  public:
    using char_type = CharType;

    output_adapter(std::vector<CharType>& vec)
        : oa(make_output_adapter<CharType>(vec)) {}

    output_adapter(std::basic_ostream<CharType>& s)
        : oa(make_output_adapter<CharType>(s)) {}

    output_adapter(StringType& s)
        : oa(make_output_adapter<CharType, StringType>(s)) {}

    /// extension point for user-defined output adapters
    explicit output_adapter(output_adapter_t<CharType> oa_)
        : oa(oa_) {}

    operator output_adapter_t<CharType>()
    {
        return oa;
    }

  private:
    output_adapter_t<CharType> oa = nullptr;
};
}  // namespace detail
}  // namespace nlohmann
