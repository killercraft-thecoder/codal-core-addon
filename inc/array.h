#ifndef ARRAY_H
#define ARRAY_H
#ifdef ENABLE_ARRAY_CLASS

#include <cstdint>
#include <cstddef>

struct Index {
    uint16_t index;
    uint8_t* data;
};

/**
 * @class Array
 * @brief A lightweight, efficient container for Index elements, designed for embedded systems.
 *
 * Provides indexed access, equality comparison, and optional safe element lookup.
 * Intended for use on STM32F4 and similar MCUs where speed and memory footprint matter.
 *
 * @note This class does not own the memory for its elements; it only stores a pointer.
 *       The caller is responsible for ensuring the lifetime of the data.
 *
 * @see Index
 */
class Array {
public:
    inline constexpr Array() noexcept : elements_(nullptr), count_(0) {}
    inline constexpr Array(Index* elements, size_t count) noexcept
        : elements_(elements), count_(count) {}

    inline ~Array() noexcept = default;

    inline constexpr size_t size() const noexcept { return count_; }

    // Mutable access
    inline Index& operator[](size_t pos) noexcept { return elements_[pos]; }
    // Read-only access
    inline const Index& operator[](size_t pos) const noexcept { return elements_[pos]; }

    inline Index* at(size_t pos) noexcept {
        return (pos < count_) ? &elements_[pos] : nullptr;
    }
    inline const Index* at(size_t pos) const noexcept {
        return (pos < count_) ? &elements_[pos] : nullptr;
    }

    inline bool operator==(const Array& other) const noexcept {
        if (count_ != other.count_) return false;
        for (size_t i = 0; i < count_; ++i) {
            if (elements_[i].index != other.elements_[i].index ||
                elements_[i].data != other.elements_[i].data)
                return false;
        }
        return true;
    }

    inline bool operator!=(const Array& other) const noexcept {
        return !(*this == other);
    }

private:
    Index* elements_;
    size_t count_;
};


#else

#include <cstdint>
#include <cstddef>

struct Index {
    uint16_t index;
    uint8_t* data;
};
/**
 * @class Array
 * @warning Unsupported in current build.
 * @note If using IntelliSense in VS Code, it may incorrectly evaluate this as unsupported
 *       simply because ENABLE_ARRAY_CLASS is not defined in the current configuration.
 *       This is a stub definition to allow code to compile without the feature enabled.
 */
class Array {
public:
    Array();                          // declared, no definition
    size_t size() const;              // declared, no definition
    ~Array();                          // declared, no definition

    // Mutable access
    Index& operator[](size_t pos);     // declared, no definition

    bool operator==(const Array& other); // declared, no definition
    bool operator!=(const Array& other); // declared, no definition
};


#endif
#endif // ARRAY_H
