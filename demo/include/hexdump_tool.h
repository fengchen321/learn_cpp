#include <iostream>
#include <iomanip>
#include <vector>

void hex_dump(const std::string &data, size_t width = 16) {
    size_t addr = 0;
    for (size_t i = 0; i < data.size(); i += width) {  // auto chunk: range | ranges::views::chunk(width) 数据分块
        std::cout << std::hex << std::setw(8) << std::setfill('0') << addr << "  ";
        for (size_t j = 0; j < width; ++j) {
            if (i + j < data.size()) {
                std::cout << std::hex << std::setw(2) << std::setfill('0');
                std::cout << static_cast<int>(static_cast<unsigned char>(data[i + j])) << " ";
            } else {
                std::cout << "   "; // 3个空格 ，setw(2)的data[i + j] + " " 
            }
        }
        std::cout << " |";
        for (size_t j = 0; j < width; ++j) {
            if (i + j < data.size()) {
                char byte = data[i + j];
                if (std::isprint(byte)) {
                    std::cout << byte;
                } else {
                    std::cout << ".";
                }
            }
        }
        std::cout << "|\n";
        addr += width;
    }
}

struct IstreamRange {
    std::istreambuf_iterator<char> b, e;

    IstreamRange(std::istream &is)
    : b(std::istreambuf_iterator<char>(is))
    , e(std::istreambuf_iterator<char>()) {}

    auto begin() const {
        return b;
    }

    auto end() const {
        return e;
    }
};

#if __has_include(<ranges>) && __has_include(<range/v3/view/chunk.hpp>) && __cplusplus >= 202002L
#include <ranges>
#include <range/v3/view/chunk.hpp>
template<class Range> requires std::ranges::range<Range>
void hex_dump(Range const & range, size_t width = 16) {
    // using T = std::decay_t<decltype(range[0])>;
    using T = std::ranges::range_value_t<Range>;
    size_t addr = 0;
    for (auto chunk : range | ranges::views::chunk(width)) { // auto chunk: range | ranges::views::chunk(width) 数据分块
        std::cout << std::setw(8) << std::setfill('0') << std::hex << addr << "  ";
        for (auto byte : chunk) {
            std::cout << std::hex <<std::setw(2 * sizeof(T)) << std::setfill('0');
            std::cout << static_cast<unsigned long long>(std::make_unsigned_t<T>(byte)) << " ";
            ++addr;
        }
        if (addr % width != 0) {
            for (size_t i = 0; i < (width -addr % width) * 3; ++i) {
                std::cout << " ";
            }
        }
        if constexpr (sizeof(T) == 1 && std::is_convertible_v<T, char>) {
            std::cout << " |";
            for (auto byte : chunk) {
                if (!std::isprint(byte)) { 
                    std::cout << ".";
                } else {
                    std::cout << byte;
                }
            }
           std::cout << "|";
        }
        std::cout << std::endl;
    }
}

void hex_dump(std::ranges::input_range auto const & range, size_t width = 16) {
    using T = std::ranges::range_value_t<decltype(range)>;
    size_t addr = 0;
    std::vector<char> saved;
    auto flags = std::cout.flags();
    for (auto chunk : range | ranges::views::chunk(width)) {
        std::cout << std::setw(8) << std::setfill('0') << std::hex << addr << "  ";
        for (auto byte : chunk) {
            std::cout << std::hex <<std::setw(2 * sizeof(T)) << std::setfill('0');
            std::cout << static_cast<std::uint64_t>(std::make_unsigned_t<T>(byte)) << " ";
            ++addr;
             if constexpr (sizeof(T) == sizeof(char) && std::is_convertible_v<T, char>) {
                saved.push_back(static_cast<char>(byte));
            }
        }
        if constexpr (sizeof(T) == sizeof(char) && std::is_convertible_v<T, char>) {
            if (addr % width != 0) {
                for (size_t i = 0; i < (width -addr % width) * 3; ++i) {
                    std::cout << " ";
                }
            }
            std::cout << " |";
            for (auto byte : saved) {
                if (!std::isprint(byte)) { 
                    byte = '.';
                }
                std::cout << byte;
            }
            std::cout << "|";
            saved.clear();
        }
       std::cout << std::endl;
    }
    std::cout.flags(flags);
}

#else
#if 0
template<typename Iterator>
constexpr bool is_random_access_iter_v = 
    std::is_base_of<std::random_access_iterator_tag,
                    typename std::iterator_traits<Iterator>::iterator_category>::value;

template<typename Iterator>
void hex_dump(Iterator begin, Iterator end, size_t width = 16) {
    using T = typename std::iterator_traits<Iterator>::value_type;
    size_t addr = 0;
    auto orign_begin = begin;

    while (begin != end) {
        std::cout << std::hex << std::setw(8) << std::setfill('0') << addr << "  ";
        for (size_t j = 0; j < width; ++j) {
            if (begin != end) {
                std::cout << std::hex << std::setw(2 * sizeof(T)) << std::setfill('0');
                std::cout << static_cast<unsigned long long>(std::make_unsigned_t<T>(*begin)) << " ";
                ++begin;
            } else {
                std::cout << "   ";
            }
        }

        if constexpr (sizeof(T) == 1 && std::is_convertible_v<T, char>) {
            std::cout << " |";
            begin = orign_begin;
            if constexpr (is_random_access_iter_v<Iterator>) {
            	begin += addr;
            }else {
            	std::advance(begin, addr);
            }
            for (size_t j = 0; j < width; ++j) {
                if (begin != end) {
                    char byte = static_cast<char>(*begin);
                    if (std::isprint(byte)) {
                        std::cout << byte;
                    } else {
                        std::cout << ".";
                    }
                    ++begin;
                }
            }
            std::cout << "|";
        }
        std::cout << std::endl;
        addr += width;
    }
}

template<typename Container>
void hex_dump(const Container &data, size_t width = 16) {
    hex_dump(std::begin(data), std::end(data), width);
}

void hex_dump(const IstreamRange &data, size_t width = 16) {
    using T = char;
    size_t addr = 0;
    std::vector<char> saved;
    auto flags = std::cout.flags();
    auto it = data.begin();
    auto end = data.end();

    while (it != end) {
        std::cout << std::hex << std::setw(8) << std::setfill('0') << addr << "  ";
        for (size_t j = 0; j < width; ++j) {
            if (it != end) {
                std::cout << std::hex << std::setw(2 * sizeof(T)) << std::setfill('0');
                std::cout << static_cast<unsigned long long>(std::make_unsigned_t<T>(*it)) << " ";
                if constexpr (sizeof(T) == 1 && std::is_convertible_v<T, char>) {
                    saved.push_back(static_cast<char>(*it));
                }
                ++it;
            } else {
                for (size_t k = 0; k < sizeof(T) * 3; ++k) {
                    std::cout << " ";
                }
            }
        }
        if constexpr (sizeof(T) == 1 && std::is_convertible_v<T, char>) {
            std::cout << " |";
            for (auto byte : saved) {
                 if (std::isprint(byte)) {
                    std::cout << byte;
                } else {
                    std::cout << ".";
                }
            }
            std::cout << "|";
        }
        saved.clear();
        std::cout << std::endl;
        addr += width;
    }
    std::cout.flags(flags);
}
#else 
template<typename Iterator>
void hex_dump(Iterator begin, Iterator end, size_t width = 16) {
    using T = typename std::iterator_traits<Iterator>::value_type;
    size_t addr = 0;
    std::vector<char> saved;
    auto flags = std::cout.flags();

    while (begin != end) {
        std::cout << std::hex << std::setw(8) << std::setfill('0') << addr << "  ";
        for (size_t j = 0; j < width; ++j) {
            if (begin != end) {
                std::cout << std::hex << std::setw(2 * sizeof(T)) << std::setfill('0');
                std::cout << static_cast<unsigned long long>(std::make_unsigned_t<T>(*begin)) << " ";
                if constexpr (sizeof(T) == 1 && std::is_convertible_v<T, char>) {
                    saved.push_back(static_cast<char>(*begin));
                }
                ++begin;
            } else {
                for (size_t k = 0; k < sizeof(T) * 3; ++k) {
                    std::cout << " ";
                }
            }
        }
        if constexpr (sizeof(T) == 1 && std::is_convertible_v<T, char>) {
            std::cout << " |";
            for (auto byte : saved) {
                 if (std::isprint(byte)) {
                    std::cout << byte;
                } else {
                    std::cout << ".";
                }
            }
            std::cout << "|";
        }
        saved.clear();
        std::cout << std::endl;
        addr += width;
    }
    std::cout.flags(flags);
}

template<typename Container>
void hex_dump(const Container &data, size_t width = 16) {
    hex_dump(std::begin(data), std::end(data), width);
}

void hex_dump(const IstreamRange &data, size_t width = 16) {
    hex_dump(data.begin(), data.end(), width);
}
#endif
#endif