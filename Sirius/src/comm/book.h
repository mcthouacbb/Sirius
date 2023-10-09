#pragma once

#include "../zobrist.h"
#include <unordered_map>
#include <vector>

struct BookEntry
{
    Move move;

    bool operator==(const BookEntry&) const = default;
    bool operator!=(const BookEntry&) const = default;
};

class Book
{
public:
    Book() = default;
    ~Book() = default;

    void loadFromPGN(const char* pgns);

    const std::vector<BookEntry>* lookup(ZKey key) const;
private:
    std::unordered_map<uint64_t, std::vector<BookEntry>> m_Entries;
};
