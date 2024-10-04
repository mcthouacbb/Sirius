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
        BOOL, // check
        INT // spin
    };

    struct IntData
    {
        int64_t value;
        int64_t defaultValue;
        int64_t minValue;
        int64_t maxValue;
    };

    struct BoolData
    {
        bool value;
    };

    UCIOption();
    UCIOption(std::string_view name, IntData data, Callback callback = Callback());
    UCIOption(std::string_view name, BoolData data, Callback callback = Callback());
    void setIntValue(int64_t value);
    void setBoolValue(bool value);

    Type type() const;
    int64_t intValue() const;
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

inline void UCIOption::setIntValue(int64_t value)
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

inline int64_t UCIOption::intValue() const
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
