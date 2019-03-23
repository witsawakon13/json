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
struct output_adapter_protocol
{
    /*!
    @brief write a character to the output
    @param[in] c  character to write
    */
    virtual void write_character(std::uint8_t c) = 0;

    /*!
    @brief write some characters to the output
    @param[in] s  pointer to a continouus chunk of memory
    @param[in] length  number of bytes to write
    */
    virtual void write_characters(const std::uint8_t* s, std::size_t length) = 0;

    virtual ~output_adapter_protocol() = default;
};

/// a type to simplify interfaces
using output_adapter_t = std::shared_ptr<output_adapter_protocol>;

/// output adapter for byte vectors
template<typename CharType>
class output_vector_adapter : public output_adapter_protocol
{
  public:
    explicit output_vector_adapter(std::vector<CharType>& vec) noexcept
        : v(vec)
    {}

    void write_character(std::uint8_t c) override
    {
        v.push_back(c);
    }

    void write_characters(const std::uint8_t* s, std::size_t length) override
    {
        std::copy(reinterpret_cast<const typename std::vector<CharType>::value_type*>(s), s + length, std::back_inserter(v));
    }

  private:
    std::vector<CharType>& v;
};

/// output adapter for output streams
template<typename CharType>
class output_stream_adapter : public output_adapter_protocol
{
  public:
    explicit output_stream_adapter(std::basic_ostream<CharType>& s) noexcept
        : stream(s)
    {}

    void write_character(std::uint8_t c) override
    {
        stream.put(static_cast<typename std::basic_ostream<CharType>::char_type>(c));
    }

    void write_characters(const std::uint8_t* s, std::size_t length) override
    {
        stream.write(reinterpret_cast<const typename std::basic_ostream<CharType>::char_type*>(s), static_cast<std::streamsize>(length));
    }

  private:
    std::basic_ostream<CharType>& stream;
};

/// output adapter for basic_string
template<typename StringType>
class output_string_adapter : public output_adapter_protocol
{
  public:
    explicit output_string_adapter(StringType& s) noexcept
        : str(s)
    {}

    void write_character(std::uint8_t c) override
    {
        str.push_back(static_cast<typename StringType::value_type>(c));
    }

    void write_characters(const std::uint8_t* s, std::size_t length) override
    {
        str.append(reinterpret_cast<const typename StringType::value_type*>(s), length);
    }

  private:
    StringType& str;
};

/*!
@brief Convenience wrapper around @ref output_adapter_protocol.
*/
class output_adapter
{
  public:
    template<typename CharType>
    output_adapter(std::vector<CharType>& vec)
        : oa(make_output_adapter<CharType>(vec)) {}

    template<typename CharType>
    output_adapter(std::basic_ostream<CharType>& s)
        : oa(make_output_adapter<CharType>(s)) {}

    template<typename CharType>
    output_adapter(std::basic_string<CharType>& s)
        : oa(make_output_adapter<CharType>(s)) {}

    /// extension point for user-defined output adapters
    explicit output_adapter(output_adapter_t oa_)
        : oa(std::move(oa_)) {}

    template<typename CharType>
    static output_adapter_t make_output_adapter(std::vector<CharType>& vec)
    {
        return std::make_shared<output_vector_adapter<CharType>>(vec);
    }

    template<typename CharType>
    static output_adapter_t make_output_adapter(std::basic_ostream<CharType>& s)
    {
        return std::make_shared<output_stream_adapter<CharType>>(s);
    }

    template<typename CharType>
    static output_adapter_t make_output_adapter(std::basic_string<CharType>& s)
    {
        return std::make_shared<output_string_adapter<std::basic_string<CharType>>>(s);
    }

    template<typename StringType>
    static output_adapter_t make_output_adapter(StringType& s)
    {
        return std::make_shared<output_string_adapter<StringType>>(s);
    }

    template<typename CharType>
    static output_adapter_t make_output_adapter(output_adapter_t oa)
    {
        return oa;
    }

    operator output_adapter_t()
    {
        return oa;
    }

  private:
    output_adapter_t oa = nullptr;
};
}  // namespace detail
}  // namespace nlohmann
