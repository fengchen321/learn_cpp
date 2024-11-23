#pragma once
#include <exception>
#include <initializer_list>
#include <type_traits>

struct BadOptionalAccess : std::exception {
    BadOptionalAccess() = default;
    virtual ~BadOptionalAccess() = default;

    const char *what() const noexcept override {
        return "bad optional access";
    }
};

struct Nullopt {
    explicit Nullopt() = default;
};

inline constexpr Nullopt nullopt;

struct InPlace {
    explicit InPlace() = default;
};

inline constexpr InPlace inplace;

template <class _Tp>
struct Optional {
public:
    using value_type = _Tp;
    using pointer = _Tp *;
    using const_pointer = _Tp const *;
    using reference = _Tp &;
    using const_reference = _Tp const &;

private:
    bool m_has_value;
    union {
        value_type m_value; // will not be initialized by default
    };

public:
    Optional(value_type &&value) noexcept : m_has_value(true), m_value(std::move(value)) {}
    Optional(value_type const &value) noexcept : m_has_value(true), m_value(std::move(value)) {}
    Optional() noexcept : m_has_value(false) {}
    Optional(Nullopt) noexcept : m_has_value(false){}

    template <class ...Ts>
    explicit Optional(InPlace, Ts &&...value_args) : m_has_value(true), m_value(std::forward<Ts>(value_args)...) {}

    template <class U, class ...Ts>
    explicit Optional(InPlace, std::initializer_list<U> ilist, Ts &&...value_args) : m_has_value(true),
        m_value(ilist, std::forward<Ts>(value_args)...) {}

    Optional(Optional const &other) :m_has_value(other.m_has_value) {
        if (m_has_value) {
            new (&m_value) value_type(other.m_value); // placement-new
            // m_value = value_type(other.m_vale) -> m_value.operator=(T const &)
        }
    }

    Optional(Optional const &&other) :m_has_value(other.m_has_value) {
        if (m_has_value) {
            // other.m_value will degenerate to (const &) so you need to keep adding std::move
            // Right-valued references become left-valued when bound to an object
            new (&m_value) value_type(std::move(other.m_value)); // placement-new
        }
    }

    Optional &operator=(Nullopt) noexcept {
        if (m_has_value) {
            m_value.~value_type();
            m_has_value = false;
        }
        return *this;
    }

    Optional &operator=(value_type &&value) noexcept {
        if (m_has_value) {
            m_value.~value_type();
            m_has_value = false;
        }
        new (&m_value) value_type(std::move(value));
        m_has_value = true;
        return *this;
    }

    Optional &operator=(const_reference value) noexcept {
        if (m_has_value) {
            m_value.~value_type();
            m_has_value = false;
        }
        new (&m_value) value_type(value);
        m_has_value = true;
        return *this;
    }


    Optional &operator=(Optional const &other) noexcept {
        if (this == &other) return *this;
        if (m_has_value) {
            m_value.~value_type();
            m_has_value = false;
        }
        if (other.m_has_value) {
            new (&m_value) value_type(other.m_value);
        }
        m_has_value = other.m_has_value;
        return *this;
    }

    Optional &operator=(Optional &&other) noexcept {
        if (this == &other) return *this;
        if (m_has_value) {
            m_value.~value_type();
            m_has_value = false;
        }
        if (other.m_has_value) {
            new (&m_value) value_type(std::move(other.m_value));
            other.m_value.~value_type();
        }
        m_has_value = other.m_has_value;
        // other.m_has_value = false;  // cppreference: Note that a moved-from optional still contains a value.
        return *this;
    }

    ~Optional() noexcept {
        if (m_has_value) {
            m_value.~value_type();
        }
    }
public:
// Modifiers
    template <class ...Ts>
    void emplace(Ts && ...value_args) {
        if (m_has_value) {
            m_value.~value_type();
            m_has_value = false;
        }
        new (&m_value) value_type(std::forward<Ts>(value_args)...);
        m_has_value = true;
    }

    template <class U, class ...Ts>
    void emplace(std::initializer_list<U> ilist, Ts &&...value_args) {
        if (m_has_value) {
            m_value.~value_type();
            m_has_value = false;
        }
        new (&m_value) value_type(ilist, std::forward<Ts>(value_args)...);
        m_has_value = true;
    }

    void reset() noexcept { // ->  *this = nullopt;
        if (m_has_value) {
            m_value.~value_type();
            m_has_value = false;
        }
    }

    void swap(Optional &other) noexcept {
        if (m_has_value && other.m_has_value) {
            using std::swap; // ADL
            swap(m_value, other.m_value);
        } else if (!m_has_value && !other.m_has_value) {
          return;
        } else if (m_has_value) {
            other.emplace(std::move(m_value));
            reset();
        } else {
            emplace(std::move(other.m_value));
            other.reset();
        }
    }
public:
// Observers
    explicit operator bool() const noexcept {
        return m_has_value;
    }

    bool has_value() const {
        return m_has_value;
    }

    const_reference value() const & {
        if (!m_has_value) {
            throw BadOptionalAccess();
        }
        return m_value; 
    }

    reference value() & {
        if (!m_has_value) {
            throw BadOptionalAccess();
        }
        return m_value; 
    }

    value_type const && value() const && {
        if (!m_has_value) {
            throw BadOptionalAccess();
        }
        return std::move(m_value); 
    }

    value_type && value() && {
        if (!m_has_value) {
            throw BadOptionalAccess();
        }
        return std::move(m_value); 
    }

    value_type value_or(value_type default_value) const & {
        if (!m_has_value) {
            return default_value;
        }
        return m_value; 
    }

    value_type value_or(value_type default_value) && noexcept {
        if (!m_has_value) {
            return default_value;
        }
        return std::move(m_value); 
    }

    const_reference operator*() const & noexcept {
        return m_value;
    }

    reference operator*() & noexcept {
        return m_value;
    }

    value_type const &&operator*() const && noexcept {
        return std::move(m_value);
    }

    value_type &&operator*() && noexcept {
        return std::move(m_value);
    }
    
    const_pointer operator->() const noexcept {
        return &m_value;
    }

    pointer operator->() noexcept {
        return &m_value;
    }
public:
// Non-member functions 
    bool operator==(Nullopt) const noexcept {
        return !m_has_value;
    }

    friend bool operator==(Nullopt, Optional const &self) noexcept {
        return !self.m_has_value;
    }

    bool operator!=(Nullopt) const noexcept {
        return m_has_value;
    }

    friend bool operator!=(Nullopt, Optional const &self) noexcept {
        return self.m_has_value;
    }

    bool operator==(Optional<value_type> const &other) const noexcept {
        if (m_has_value != other.m_has_value)
            return false;
        if (m_has_value) {
            return m_value == other.m_value;
        }
        return true;
    }

    bool operator!=(Optional const &other) const noexcept {
        if (m_has_value != other.m_has_value)
            return true;
        if (m_has_value) {
            return m_value != other.m_value;
        }
        return false;
    }

    bool operator>(Optional const &other) const noexcept {
        if (!m_has_value || !other.m_has_value)
            return false;
        return m_value > other.m_value;
    }

    bool operator<(Optional const &other) const noexcept {
        if (!m_has_value || !other.m_has_value)
            return false;
        return m_value < other.m_value;
    }

    bool operator>=(Optional const &other) const noexcept {
        if (!m_has_value || !other.m_has_value)
            return true;
        return m_value >= other.m_value;
    }

    bool operator<=(Optional const &other) const noexcept {
        if (!m_has_value || !other.m_has_value)
            return true;
        return m_value <= other.m_value;
    }

// Monadic operations
    template <class F>
    auto and_then(F &&f) const &
        -> typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(m_value))>::type>::type {
        if (m_has_value) {
            return std::forward<F>(f)(m_value);
        } else {
            return typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(m_value))>::type>::type{};
        }
    }

    template <class F>
    auto and_then(F &&f) &
        -> typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(m_value))>::type>::type {
        if (m_has_value) {
            return std::forward<F>(f)(m_value);
        } else {
            return typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(m_value))>::type>::type{};
        }
    }

    template <class F>
    auto and_then(F &&f) const &&
        -> typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(std::move(m_value)))>::type>::type {
        if (m_has_value) {
            return std::forward<F>(f)(std::move(m_value));
        } else {
            return typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(std::move(m_value)))>::type>::type{};
        }
    }

    template <class F>
    auto and_then(F &&f) &&
        -> typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(std::move(m_value)))>::type>::type {
        if (m_has_value) {
            return std::forward<F>(f)(std::move(m_value));
        } else {
            return typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(std::move(m_value)))>::type>::type{};
        }
    }

    template <class F>
    auto transform(F &&f) const &
        -> Optional<typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(m_value))>::type>::type> {
        if (m_has_value) {
            return std::forward<F>(f)(m_value);
        } else {
            return nullopt;
        }
    }

    template <class F>
    auto transform(F &&f) &
        -> Optional<typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(m_value))>::type>::type> {
        if (m_has_value) {
            return std::forward<F>(f)(m_value);
        } else {
            return nullopt;
        }
    }

    template <class F>
    auto transform(F &&f) const &&
        -> Optional<typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(std::move(m_value)))>::type>::type> {
        if (m_has_value) {
            return std::forward<F>(f)(std::move(m_value));
        } else {
            return nullopt;
        }
    }

    template <class F>
    auto transform(F &&f) &&
        -> Optional<typename std::remove_cv<
            typename std::remove_reference<
                decltype(f(std::move(m_value)))>::type>::type> {
        if (m_has_value) {
            return std::forward<F>(f)(std::move(m_value));
        } else {
            return nullopt;
        }
    }

    template <class F, typename std::enable_if<std::is_copy_constructible<value_type>::value, int>::type = 0>
    Optional or_else(F &&f) const & {
        if (m_has_value) {
            return *this;
        } else {
            return std::forward<F>(f)();
        }
    }

    template <class F, typename std::enable_if<std::is_move_constructible<value_type>::value, int>::type = 0>
    Optional or_else(F &&f) && {
        if (m_has_value) {
            return std::move(*this);
        } else {
            return std::forward<F>(f)();
        }
    }
};

#if __cpp_deduction_guides
template <class T> //  CTAD
Optional(T) -> Optional<T>;
#endif

template <class T>
Optional<T> makeOptional(T value) {
    return Optional<T>(std::move(value));
}