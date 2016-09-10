#pragma once

#include <cassert>
#include <experimental/type_traits>
#include "xson/fast/encoder.hpp"
#include "xson/object.hpp"

namespace xson::fson {

using std::enable_if_t;

using std::experimental::is_enum_v;

class encoder : public fast::encoder
{
public:

    encoder(std::ostream& os) : fast::encoder{os}
    {}

    using fast::encoder::encode;

    template<typename T,
             typename = enable_if_t<is_enum_v<T>>>
    void encode(T e)
    {
        encode(static_cast<std::uint8_t>(e));
    }

    void encode(std::double_t d)
    {
        union {
            std::double_t d64;
            std::uint64_t i64;
        } d2i;
        d2i.d64 = d;
        encode(d2i.i64);
    }

    void encode(std::bool_t b)
    {
        if(b)
            encode(std::uint8_t{'\x01'});
        else
            encode(std::uint8_t{'\x00'});
    }

    void encode(const std::datetime_t dt)
    {
        using namespace std::chrono;
        const auto us = duration_cast<milliseconds>(dt.time_since_epoch());
        encode(static_cast<std::uint64_t>(us.count()));
    }

    void encode(const xson::object& obj)
    {
        switch(obj.type())
        {
            case type::object:
            case type::array:
            for(const auto& o : obj)
            {
                encode(o.second.type()); // type
                encode(o.first);         // name
                encode(o.second);        // object
            }
            encode(type::eod);
            break;

            case type::int32:
            encode(get<std::int32_t>(obj.value()));
            break;

            case type::int64:
            encode(get<std::int64_t>(obj.value()));
            break;

            case type::number:
            encode(get<std::double_t>(obj.value()));
            break;

            case type::string:
            encode(get<std::string_t>(obj.value()));
            break;

            case type::boolean:
            encode(get<std::bool_t>(obj.value()));
            break;

            case type::date:
            encode(get<std::datetime_t>(obj.value()));
            break;

            case type::null:
            break;

            case type::eod:
            assert(false);
            break;
        }
    }

};

inline auto& operator << (std::ostream& os, const xson::object& ob)
{
    xson::fson::encoder{os}.encode(ob);
    return os;
}

} // namespace xson::fson
