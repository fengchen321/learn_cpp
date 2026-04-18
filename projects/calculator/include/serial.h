#pragma once

#include <string>
#include <fstream>
#include <cstdint>
#include "exception.h"

class Serializer {
public:
    explicit Serializer(const std::string& filename)
        : ofs_(filename, std::ios::out | std::ios::binary) {
        if (!ofs_) {
            throw RuntimeError("Failed to open file for writing: " + filename);
        }
    }

    ~Serializer() = default;

    // Disable copy
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;

    // Allow move
    Serializer(Serializer&&) noexcept = default;
    Serializer& operator=(Serializer&&) noexcept = default;

    // Generic put for primitive types
    template <typename T>
    Serializer& put(const T& value) {
        ofs_.write(reinterpret_cast<const char*>(&value), sizeof(T));
        if (!ofs_) {
            throw RuntimeError("Failed to write to file");
        }
        return *this;
    }

    // Specialization for std::string: write length + content
    Serializer& putString(const std::string& value) {
        uint32_t len = static_cast<uint32_t>(value.size());
        put(len);
        if (len > 0) {
            ofs_.write(value.data(), len);
            if (!ofs_) {
                throw RuntimeError("Failed to write string to file");
            }
        }
        return *this;
    }

    // Specialization for bool: write as uint8_t for portability
    Serializer& putBool(bool value) {
        uint8_t b = value ? 1 : 0;
        return put(b);
    }

    template <typename T>
    Serializer& operator<<(const T& value) {
        return put(value);
    }

    Serializer& operator<<(const std::string& value) {
        return putString(value);
    }

    Serializer& operator<<(bool value) {
        return putBool(value);
    }

private:
    std::ofstream ofs_;
};

class DeSerializer {
public:
    explicit DeSerializer(const std::string& filename)
        : ifs_(filename, std::ios::in | std::ios::binary) {
        if (!ifs_) {
            throw RuntimeError("Failed to open file for reading: " + filename);
        }
    }

    ~DeSerializer() = default;

    // Disable copy
    DeSerializer(const DeSerializer&) = delete;
    DeSerializer& operator=(const DeSerializer&) = delete;

    // Allow move
    DeSerializer(DeSerializer&&) noexcept = default;
    DeSerializer& operator=(DeSerializer&&) noexcept = default;

    // Generic get for primitive types
    template <typename T>
    DeSerializer& get(T& value) {
        ifs_.read(reinterpret_cast<char*>(&value), sizeof(T));
        if (!ifs_) {
            throw RuntimeError("Failed to read from file");
        }
        return *this;
    }

    // Specialization for std::string: read length + content
    DeSerializer& getString(std::string& value) {
        uint32_t len;
        get(len);
        if (len > 0) {
            value.resize(len);
            ifs_.read(value.data(), len);
            if (!ifs_) {
                throw RuntimeError("Failed to read string from file");
            }
        } else {
            value.clear();
        }
        return *this;
    }

    // Specialization for bool: read as uint8_t
    DeSerializer& getBool(bool& value) {
        uint8_t b;
        get(b);
        value = (b != 0);
        return *this;
    }

    template <typename T>
    DeSerializer& operator>>(T& value) {
        return get(value);
    }

    DeSerializer& operator>>(std::string& value) {
        return getString(value);
    }

    DeSerializer& operator>>(bool& value) {
        return getBool(value);
    }

private:
    std::ifstream ifs_;
};

class Serializable {
public:
    virtual void serialize(Serializer& output) const = 0;
    virtual void deserialize(DeSerializer& input) = 0;
    virtual ~Serializable() = default;
};