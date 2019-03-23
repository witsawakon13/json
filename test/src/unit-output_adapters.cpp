/*
    __ _____ _____ _____
 __|  |   __|     |   | |  JSON for Modern C++ (test suite)
|  |  |__   |  |  | | | |  version 3.6.1
|_____|_____|_____|_|___|  https://github.com/nlohmann/json

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2013-2019 Niels Lohmann <http://nlohmann.me>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "catch.hpp"

#include <memory>
#include <nlohmann/json.hpp>
using nlohmann::json;

class my_output_adapter : public nlohmann::detail::output_adapter_protocol
{
  public:
    void write_character(std::uint8_t c) final
    {
        result += std::to_string(c) + " ";
    }

    void write_characters(const std::uint8_t* s, std::size_t length) final
    {
        for (std::size_t i = 0; i < length; ++i)
        {
            result += std::to_string(s[i]) + " ";
        }
    }

    std::string result;
};

TEST_CASE("user-defined output adapters")
{
    SECTION("binary formats")
    {
        json j = {{"foo", {1, 2, 3}}};

        SECTION("CBOR")
        {
            auto o = std::make_shared<my_output_adapter>();
            json::to_cbor(j, nlohmann::detail::output_adapter(o));
            CHECK(o->result == "161 99 102 111 111 131 1 2 3 ");
        }

        SECTION("MessagePack")
        {
            auto o = std::make_shared<my_output_adapter>();
            json::to_msgpack(j, nlohmann::detail::output_adapter(o));
            CHECK(o->result == "129 163 102 111 111 147 1 2 3 ");
        }

        SECTION("UBJSON")
        {
            auto o = std::make_shared<my_output_adapter>();
            json::to_ubjson(j, nlohmann::detail::output_adapter(o));
            CHECK(o->result == "123 105 3 102 111 111 91 105 1 105 2 105 3 93 125 ");
        }

        SECTION("BSON")
        {
            auto o = std::make_shared<my_output_adapter>();
            json::to_bson(j, nlohmann::detail::output_adapter(o));
            CHECK(o->result == "36 0 0 0 4 102 111 111 0 26 0 0 0 16 48 0 1 0 0 0 16 49 0 2 0 0 0 16 50 0 3 0 0 0 0 0 ");
        }
    }
}
