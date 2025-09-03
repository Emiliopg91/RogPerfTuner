#pragma once

template <typename Derived, typename EnumType, size_t N>
class StrEnum
{
public:
    using Enum = EnumType;

    StrEnum(Enum e) : value(e) {}
    StrEnum(const std::string &str) : value(fromString(str)) {}

    std::string toString() const
    {
        const auto &table = Derived::metaTable();
        for (size_t i = 0; i < N; ++i)
            if (table[i].e == value)
                return table[i].val;
        return "UNKNOWN";
    }

    std::string toName() const
    {
        const auto &table = Derived::metaTable();
        for (size_t i = 0; i < N; ++i)
            if (table[i].e == value)
                return table[i].name;
        return "UNKNOWN";
    }

    bool operator==(const StrEnum &other) const { return value == other.value; }
    bool operator!=(const StrEnum &other) const { return value != other.value; }

    friend std::ostream &operator<<(std::ostream &os, const StrEnum &obj)
    {
        os << obj.toName() << "(" << obj.toString() << ")";
        return os;
    }

    const static Enum &fromString(const std::string &v)
    {
        const auto &table = Derived::metaTable();
        for (size_t i = 0; i < N; ++i)
            if (table[i].val == v)
                return table[i].e;
        throw std::invalid_argument("Unknown value: " + v);
    }

    static std::vector<Enum> getAll()
    {
        std::vector<Enum> all;

        const auto &table = Derived::metaTable();
        for (size_t i = 0; i < N; ++i)
            all.push_back(table[i].e);

        return all;
    }

protected:
    Enum value;
};
