#pragma once

#include <variant>
#include <string>
#include <string_view>
#include <functional>
#include <cassert>


// currently only supports integers
class UCIOption
{
public:
    using Callback = std::function<void(const UCIOption&)>;
    enum class Type
    {
        NONE,
        INT // spin
    };

    struct IntData
    {
        int64_t value;
        int64_t defaultValue;
        int64_t minValue;
        int64_t maxValue;
    };

    UCIOption();
    UCIOption(std::string_view name, IntData data, Callback callback = Callback());
    UCIOption& operator=(int64_t value);

    Type type() const;
    int64_t intValue() const;
    const IntData& intData() const;
    const std::string& name() const;
private:
    Type m_Type;
    std::string m_Name;
    Callback m_Callback;
    std::variant<IntData> m_Data;
};

inline UCIOption::UCIOption()
    : m_Type(Type::NONE)
{

}

inline UCIOption::UCIOption(std::string_view name, IntData data, Callback callback)
    : m_Type(Type::INT), m_Name(name), m_Callback(callback), m_Data(data)
{

}

inline UCIOption& UCIOption::operator=(int64_t value)
{
    assert(m_Type == Type::INT);
    std::get<IntData>(m_Data).value = value;
    if (m_Callback)
        m_Callback(*this);
    return *this;
}

inline UCIOption::Type UCIOption::type() const
{
    return m_Type;
}

inline int64_t UCIOption::intValue() const
{
    assert(m_Type == Type::INT);
    return std::get<IntData>(m_Data).value;
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
