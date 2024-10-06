#pragma once
#include <cstddef> // size_t
#include <stdexcept> // std::out_of_range
#include <iterator> // std::reverse_iterator
#include <algorithm> // std::equal
#include "_Common.h"
// C++ 标准规定：单下划线+大写字母（_Identifier）或 双下划线+小写字母（__identifier）的标识符是保留字。理论上用户不得使用，只允许标准库和编译器使用。

template <class _Tp, size_t _N>
struct Array {
    using value_type = _Tp;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = _Tp *;
    using const_pointer = _Tp const *;
    using reference = _Tp &;
    using const_reference = _Tp const &;
    using iterator = _Tp *;
    using const_iterator = _Tp const *;
    using reverse_iterator = std::reverse_iterator<_Tp *>;
    using const_reverse_iterator = std::reverse_iterator<_Tp const *>;

    value_type _M_elements[_N];

    // Element access.
    reference operator[](size_type __i) noexcept {
        return _M_elements[__i];
    }

    const_reference operator[](size_type __i) const noexcept {
        return _M_elements[__i];
    }

    reference at(size_type __i) {
        if (__i >= _N) throw std::out_of_range("Array::at");
        return _M_elements[__i];
    }

    const_reference at(size_type __i) const {
        if (__i >= _N) throw std::out_of_range("Array::at");
        return _M_elements[__i];
    }
    // std::is_nothrow_copy_assignable_v<_Tp>
    void fill(const_reference __val) noexcept(noexcept(_M_elements[0] = __val)){
        for (size_type __i = 0; __i < _N; ++__i) {
            _M_elements[__i] = __val;
        }
    }
    // std::is_nothrow_swappable_v<_Tp>
    void swap(Array &__other) noexcept(noexcept(std::swap(_M_elements[0], __other._M_elements[0]))) {
        for (size_type __i = 0; __i < _N; ++__i) {
            std::swap(_M_elements[__i], __other._M_elements[__i]);
        }
    }

    reference front() noexcept {
        return _M_elements[0];
    }

    const_reference front() const noexcept {
        return _M_elements[0];
    }

    reference back() noexcept {
        return _M_elements[_N - 1];
    }

    const_reference back() const noexcept {
        return _M_elements[_N - 1];
    }

    // Capacity.
    static constexpr bool empty() noexcept {
        return false;
    }

    static constexpr size_type size() noexcept {
        return _N;
    }

    static constexpr size_type max_size() noexcept {
        return _N;
    }

    pointer data() noexcept {
        return _M_elements;
    }

    const_pointer data() const noexcept {
        return _M_elements;
    }

    pointer cdata() noexcept {
        return _M_elements;
    }

    const_pointer cdata() const noexcept {
        return _M_elements;
    }

    // Iterators.
    iterator begin() noexcept {
        return _M_elements;
    }

    const_iterator begin() const noexcept {
        return _M_elements;
    }

    iterator end() noexcept {
        return _M_elements + _N;
    }

    const_iterator end() const noexcept {
        return _M_elements + _N;
    }

    iterator cbegin() noexcept {
        return _M_elements;
    }

    const_iterator cbegin() const noexcept {
        return _M_elements;
    }

    iterator cend() noexcept {
        return _M_elements + _N;
    }

    const_iterator cend() const noexcept {
        return _M_elements + _N;
    }
    
	reverse_iterator crbegin() noexcept {
        return std::make_reverse_iterator(_M_elements + _N);
    }

    const_reverse_iterator crbegin() const noexcept {
        return std::make_reverse_iterator(_M_elements + _N);
    }

    reverse_iterator crend() noexcept {
        return std::make_reverse_iterator(_M_elements);
    }

    const_reverse_iterator crend() const noexcept {
        return std::make_reverse_iterator(_M_elements);
    }

    reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(_M_elements + _N);
    }

    const_reverse_iterator rbegin() const noexcept {
        return std::make_reverse_iterator(_M_elements + _N);
    }

    reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(_M_elements);
    }

    const_reverse_iterator rend() const noexcept {
        return std::make_reverse_iterator(_M_elements);
    }

    _LIBPENGCXX_DEFINE_COMPARISON(Array);
};

template <class _Tp>
struct Array<_Tp, 0> {
    using value_type = _Tp;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = _Tp *;
    using const_pointer = _Tp const *;
    using reference = _Tp &;
    using const_reference = _Tp const &;
    using iterator = std::nullptr_t;
    using const_iterator = std::nullptr_t;
    using reverse_iterator = std::nullptr_t;
    using const_reverse_iterator = std::nullptr_t;

    // Element access.
    reference operator[](size_type __i) noexcept {
        _LIBPENGCXX_UNREACHABLE();
    }

    const_reference operator[](size_type __i) const noexcept {
        _LIBPENGCXX_UNREACHABLE();
    }

    reference at(size_type __i) {
        throw std::out_of_range("Array::at");
    }

    const_reference at(size_type __i) const {
        throw std::out_of_range("Array::at");
    }

    void fill(const_reference) noexcept {
    }

    void swap(Array &) noexcept {
    }

    reference front() noexcept {
        _LIBPENGCXX_UNREACHABLE();
    }

    const_reference front() const noexcept {
        _LIBPENGCXX_UNREACHABLE();
    }

    reference back() noexcept {
        _LIBPENGCXX_UNREACHABLE();
    }

    const_reference back() const noexcept {
        _LIBPENGCXX_UNREACHABLE();
    }

    // Capacity.
    static constexpr bool empty() noexcept {
        return true;
    }

    static constexpr size_type size() noexcept {
        return 0;
    }

    static constexpr size_type max_size() noexcept {
        return 0;
    }

    pointer data() noexcept {
        return nullptr;
    }

    const_pointer data() const noexcept {
        return nullptr;
    }

    pointer cdata() noexcept {
        return nullptr;
    }

    const_pointer cdata() const noexcept {
        return nullptr;
    }

    // Iterators.
    iterator begin() noexcept {
        return nullptr;
    }

    const_iterator begin() const noexcept {
        return nullptr;
    }

    iterator end() noexcept {
        return nullptr;
    }

    const_iterator end() const noexcept {
        return nullptr;
    }

    iterator cbegin() noexcept {
        return nullptr;
    }

    const_iterator cbegin() const noexcept {
        return nullptr;
    }

    iterator cend() noexcept {
        return nullptr;
    }

    const_iterator cend() const noexcept {
        return nullptr;
    }
    
	reverse_iterator crbegin() noexcept {
        return nullptr;
    }

    const_reverse_iterator crbegin() const noexcept {
        return nullptr;
    }

    reverse_iterator crend() noexcept {
        return nullptr;
    }

    const_reverse_iterator crend() const noexcept {
        return nullptr;
    }

    reverse_iterator rbegin() noexcept {
        return nullptr;
    }

    const_reverse_iterator rbegin() const noexcept {
        return nullptr;
    }

    reverse_iterator rend() noexcept {
        return nullptr;
    }

    const_reverse_iterator rend() const noexcept {
        return nullptr;
    }

    _LIBPENGCXX_DEFINE_COMPARISON(Array);
};

template<class _Tp, class ..._Ts>
Array(_Tp, _Ts...) -> Array<_Tp, 1 + sizeof...(_Ts)>;

template <typename T, std::size_t... I>
Array<T, sizeof...(I)> make_array_impl(T (&arr)[sizeof...(I)], std::index_sequence<I...>) {
    return Array<T, sizeof...(I)>{ { arr[I]... } };
}

template <typename T, std::size_t N>
Array<T, N> make_array(T (&arr)[N]) {
    return make_array_impl(arr, std::make_index_sequence<N>{});
}

template <typename T>
Array<T, 0> make_array(T (&)[0]) {
    return Array<T, 0>{};
}