#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <string_view>
#include <variant>

namespace uci
{

// currently only supports integers
class UCIOption
{
public:
    using Callback = std::function<void(const UCIOption&)>;
    enum class Type
    {
        NONE,
        BOOL, // check
        INT // spin
    };

    struct IntData
    {
        i64 value;
        i64 defaultValue;
        i64 minValue;
        i64 maxValue;
    };

    struct BoolData
    {
        bool value;
    };

    UCIOption();
    UCIOption(std::string_view name, IntData data, Callback callback = Callback());
    UCIOption(std::string_view name, BoolData data, Callback callback = Callback());
    void setIntValue(i64 value);
    void setBoolValue(bool value);

    Type type() const;
    i64 intValue() const;
    bool boolValue() const;
    const IntData& intData() const;
    const std::string& name() const;

private:
    Type m_Type;
    std::string m_Name;
    Callback m_Callback;
    std::variant<IntData, BoolData> m_Data;
};

inline UCIOption::UCIOption()
    : m_Type(Type::NONE)
{
}

inline UCIOption::UCIOption(std::string_view name, IntData data, Callback callback)
    : m_Type(Type::INT), m_Name(name), m_Callback(callback), m_Data(data)
{
}

inline UCIOption::UCIOption(std::string_view name, BoolData data, Callback callback)
    : m_Type(Type::BOOL), m_Name(name), m_Callback(callback), m_Data(data)
{
}

inline void UCIOption::setIntValue(i64 value)
{
    assert(m_Type == Type::INT);
    std::get<IntData>(m_Data).value = value;
    if (m_Callback)
        m_Callback(*this);
}

inline void UCIOption::setBoolValue(bool value)
{
    assert(m_Type == Type::BOOL);
    std::get<BoolData>(m_Data).value = value;
    if (m_Callback)
        m_Callback(*this);
}

inline UCIOption::Type UCIOption::type() const
{
    return m_Type;
}

inline i64 UCIOption::intValue() const
{
    assert(m_Type == Type::INT);
    return std::get<IntData>(m_Data).value;
}

inline bool UCIOption::boolValue() const
{
    assert(m_Type == Type::BOOL);
    return std::get<BoolData>(m_Data).value;
}

inline const UCIOption::IntData& UCIOption::intData() const
{
    assert(m_Type == Type::INT);
    return std::get<IntData>(m_Data);
}

inline const std::string& UCIOption::name() const
{
    return m_Name;
}

}
