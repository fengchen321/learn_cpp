#pragma once
#include <cstddef> // size_t
#include <stdexcept> // std::out_of_range
#include <iterator> // std::reverse_iterator
#include <algorithm> // std::equal
#include <memory>
#include <initializer_list>
#include "_Common.h"

template <class _Tp, class _Alloc = std::allocator<_Tp>>
struct Vector {
public:
    using value_type = _Tp;
    using allocator_type = _Alloc;
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

private:
    pointer _M_data;
    size_type _M_size;
    size_type _M_capacity;
    allocator_type _M_alloc;
    
public:
    Vector() : _M_data(nullptr), _M_size(0), _M_capacity(0), _M_alloc() {}

    explicit Vector(size_type __n, const allocator_type& __a = allocator_type()) {
        _M_data = _M_alloc.allocate(__n);
        _M_size = __n;
        _M_capacity = __n;
        for (size_type __i = 0; __i < __n; ++__i) {
            _M_alloc.construct(&_M_data[__i]);
        }
        
    }

    Vector(size_type __n, const_reference value, const allocator_type& __a = allocator_type()) {
        _M_data = _M_alloc.allocate(__n);
        _M_size = __n;
        _M_capacity = __n;
        for (size_type __i = 0; __i < __n; ++__i) {
            _M_alloc.construct(&_M_data[__i], value);
        }
    }

    template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
	Vector(_InputIterator __first, _InputIterator __last, const allocator_type& __a = allocator_type()) {
        size_type __n = __last - __first;
        _M_data = _M_alloc.allocate(__n);
        _M_size = __n;
        _M_capacity = __n;
        for (size_type __i = 0; __i < __n; ++__i) {
            _M_alloc.construct(&_M_data[__i], *__first);
            ++__first;
        }
    }

    Vector(std::initializer_list<value_type> __l, const allocator_type& __a = allocator_type()) 
     : Vector(__l.begin(), __l.end(), __a) {}

    Vector(Vector const& __other) {
        _M_size = __other._M_size;
        _M_capacity = __other._M_capacity;
        if (_M_size > 0) {
            _M_data = _M_alloc.allocate(_M_size);
            for (std::size_t __i = 0; __i < _M_size; ++__i) {
                _M_alloc.construct(&_M_data[__i], __other._M_data[__i]);
                // std::allocator_traits<allocator_type>::construct(_M_alloc, &_M_data[__i], __other._M_data[__i]);
                // std::construct_at(&_M_data[__i], std::as_const(__other._M_data[__i])); // c++20
            }
            // std::construct(_M_data, __other._M_data, __other._M_data + __other._M_size);    // c++20       
        } else {
            _M_data = nullptr;
        }
    }

    Vector &operator=(Vector const& __other) {
        if (this != &__other) {
            _M_size = __other._M_size;
            _M_capacity = __other._M_capacity;
            if (_M_size > 0) {
                _M_data = _M_alloc.allocate(__other._M_size);
                std::uninitialized_copy(__other._M_data, __other._M_data + __other._M_size, _M_data);
                
            } else {
                _M_data = nullptr;
            }
        }
        return *this;
    }

    void assign(size_type __n, const value_type& __val) {
        clear();
        reserve(__n);
        for (size_type __i = 0; __i < __n; ++__i) {
            _M_alloc.construct(&_M_data[__i], __val);
        }
    }

    template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
	void assign(_InputIterator __first, _InputIterator __last) {
        clear();
        reserve(__last - __first);
        std::uninitialized_copy(__first, __last, _M_data);
    }
    
    void assign(std::initializer_list<value_type> __l) {
        assign(__l.begin(), __l.end());
    }

    Vector &operator=(std::initializer_list<value_type> __l) {
        assign(__l.begin(), __l.end());
        return *this;
    }

    Vector(Vector && __other) noexcept : _M_alloc(std::move(__other._M_alloc)) {
        _M_data = __other._M_data;
        _M_size = __other._M_size;
        _M_capacity = __other._M_capacity;
        __other._M_data = nullptr;
        __other._M_size = 0;
        __other._M_capacity = 0;
    }

    Vector &operator=(Vector && __other) noexcept {
        if (this != &__other) {
            for (size_type __i = 0; __i < _M_size; ++__i) {
                std::destroy_at(&_M_data[__i]);
            }
            if (_M_data) {
                _M_alloc.deallocate(_M_data, _M_capacity);
            }
            _M_data = __other._M_data;
            _M_size = __other._M_size;
            _M_capacity = __other._M_capacity;
            __other._M_data = nullptr;
            __other._M_size = 0;
            __other._M_capacity = 0;
        }
        return *this;
    }

    ~Vector() {
        for (std::size_t __i = 0; __i != _M_size; __i++) {
            std::destroy_at(&_M_data[__i]);
        }
        if (_M_data) {
            _M_alloc.deallocate(_M_data, _M_capacity);
        }
    }

    allocator_type get_allocater() const noexcept {
        return _M_alloc;
    }

public:
    // Element access.
    reference operator[](size_type __i) noexcept {
        return _M_data[__i];
    }

    const_reference operator[](size_type __i) const noexcept {
        return _M_data[__i];
    }

    reference at(size_type __i) {
        if (__i >= _M_size) throw std::out_of_range("Vector::at");
        return _M_data[__i];
    }

    const_reference at(size_type __i) const {
        if (__i >= _M_size) throw std::out_of_range("Vector::at");
        return _M_data[__i];
    }

    reference front() noexcept {
        return *_M_data;
    }

    const_reference front() const noexcept {
        return *_M_data;
    }

    reference back() noexcept {
        return _M_data[_M_size - 1];
    }

    const_reference back() const noexcept {
        return _M_data[_M_size - 1];
    }

    pointer data() noexcept {
        return _M_data;
    }

    const_pointer data() const noexcept {
        return _M_data;
    }

    pointer cdata() noexcept {
        return _M_data;
    }

    const_pointer cdata() const noexcept {
        return _M_data;
    }

    // Capacity.
    bool empty() const noexcept {
        return _M_size == 0;
    }

    size_type size() const noexcept {
        return _M_size;
    }

    static constexpr size_type max_size() noexcept {
        const size_t __diffmax = std::numeric_limits<ptrdiff_t>::max() / sizeof(value_type);
        const size_t __allocmax = allocator_type().max_size();
	    return (std::min)(__diffmax, __allocmax);
    }

    void reserve(size_type __n) {
        if (__n <= _M_capacity) return;
        __n = std::max(__n, 2 * _M_capacity);
        // printf("Reserve from %zu to %zu\n", _M_capacity, __n);
        pointer old_M_data = _M_data;
        size_type old_M_capacity= _M_capacity;
        if (__n == 0) {
            _M_data = nullptr;
            _M_capacity = 0;
        } else {
            _M_data = _M_alloc.allocate(__n);
            _M_capacity = __n;
        }
        if (old_M_data) {
            for (size_type __i = 0; __i < _M_size; ++__i) {
                _M_alloc.construct(_M_data + __i, std::move(old_M_data[__i]));
            }
            // std::uninitialized_copy(old_M_data, old_M_data + _M_size, _M_data);
            for (std::size_t __i = 0; __i < _M_size; ++__i) {
                std::destroy_at(old_M_data + __i);
            }
            _M_alloc.deallocate(old_M_data, old_M_capacity);
        }
    }

    size_type capacity() const noexcept {
        return _M_capacity;
    }

    void shrink_to_fit() noexcept {
        pointer old_M_data = _M_data;
        if (_M_size == 0) {
            _M_data = nullptr;
            _M_capacity = 0;
        } else {
            _M_data = _M_alloc.allocate(_M_size);
            _M_capacity = _M_size;
        }
        if (old_M_data) {
            std::uninitialized_copy(old_M_data, old_M_data + _M_size, _M_data);
            for (std::size_t __i = 0; __i < _M_size; ++__i) {
                std::destroy_at(old_M_data + __i);
            }
            _M_alloc.deallocate(old_M_data, _M_capacity);
        }
    }

    // Modifiers
    void clear() noexcept {
        for (std::size_t __i = 0; __i < _M_size; ++__i) {
            std::destroy_at(&_M_data[__i]);
        }
        // std::destory(_M_data, _M_data + _M_size)
        _M_size = 0;
    }

    iterator insert(const_iterator __position, const value_type& __x) {
        size_type __i = std::distance(static_cast<const_iterator>(begin()),__position);
        reserve(_M_size + 1);
        for (size_type __j = _M_size; __j > __i; --__j) {
            _M_alloc.construct(&_M_data[__j], std::move(_M_data[__j - 1]));
            // _M_data[__j] = std::move(_M_data[__j - 1]);
            std::destroy_at(&_M_data[__j - 1]);
        }
        _M_size += 1;
        _M_alloc.construct(&_M_data[__i], __x);
        return _M_data + __i;
    }

    iterator insert(const_iterator __position, value_type&& __x) {
        size_type __i = std::distance(static_cast<const_iterator>(begin()),__position);
        reserve(_M_size + 1);
        for (size_type __j = _M_size; __j > __i; --__j) {
            _M_alloc.construct(&_M_data[__j], std::move(_M_data[__j - 1]));
            // _M_data[__j] = std::move(_M_data[__j - 1]);
            std::destroy_at(&_M_data[__j - 1]);
        }
        _M_size += 1;
        _M_alloc.construct(&_M_data[__i], std::move(__x));
        return _M_data + __i;
    }

    iterator insert(const_iterator __position, size_type __n, const value_type& __x) {
        size_type __i = std::distance(static_cast<const_iterator>(begin()),__position);
        if (__n == 0) return _M_data + __i;
        reserve(_M_size + __n);
        for (size_type __j = _M_size; __j > __i; --__j) {
            _M_alloc.construct(&_M_data[__j + __n - 1], std::move(_M_data[__j - 1]));
            // _M_data[__j + __n - 1] = std::move(_M_data[__j - 1]);
            std::destroy_at(&_M_data[__j - 1]);
        }
        _M_size += __n;
        for (size_type __k = __i; __k < __i + __n; ++__k) {
            _M_alloc.construct(&_M_data[__k], __x);
        }
        return _M_data + __i;
    }

    template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
	iterator insert(const_iterator __position, _InputIterator __first, _InputIterator __last) {
        size_type __i = std::distance(static_cast<const_iterator>(begin()),__position);
        size_type __n = std::distance(__first, __last);
        if (__n == 0) return _M_data + __i;
        reserve(_M_size + __n);
        for (size_type __j = _M_size; __j > __i; --__j) {
             _M_alloc.construct(&_M_data[__j + __n - 1], std::move(_M_data[__j - 1]));
            // _M_data[__j + __n - 1] = std::move(_M_data[__j - 1]);
            std::destroy_at(&_M_data[__j - 1]);
        }
        _M_size += __n;
        for (size_type __k = __i; __k < __i + __n; ++__k) {
            _M_alloc.construct(&_M_data[__k], *__first++);
        }
        return _M_data + __i;
    }

    iterator insert(const_iterator __position, std::initializer_list<value_type> __l) {
        return insert(__position, __l.begin(), __l.end());
    }

    template <class ..._Args>
    iterator emplace(const_iterator __position, _Args&&... __args) {
        size_type __i = std::distance(static_cast<const_iterator>(begin()), __position);
        reserve(_M_size + 1);
        for (size_type __j = _M_size; __j > __i; --__j) {
             _M_alloc.construct(&_M_data[__j], std::move(_M_data[__j - 1]));
            // _M_data[__j] = std::move(_M_data[__j - 1]);
            std::destroy_at(&_M_data[__j - 1]);
        }
        _M_size += 1;
        _M_alloc.construct(&_M_data[__i], std::forward<_Args>(__args)...);
        return _M_data + __i;
    }

    iterator erase(iterator __pos) noexcept {
        size_type __i = std::distance(begin(), __pos);
        for (size_type __j = __i + 1; __j < _M_size; ++__j) {
            _M_data[__j - 1] = std::move(_M_data[__j]);
        }
        --_M_size;
        std::destroy_at(&_M_data[_M_size]);
        return __pos;
    }

    iterator erase(const_iterator __first, const_iterator __last) {
        size_type __diff = std::distance(__first,  __last);
        for (size_type __j = __last - begin(); __j < _M_size; ++__j) {
            _M_data[__j - __diff] = std::move(_M_data[__j]);
        }
        _M_size -= __diff;
        for (size_type __j = _M_size; __j < _M_size + __diff; ++__j) {
            std::destroy_at(&_M_data[__j]);
        }
        return const_cast<iterator>(__first);
    }

    void push_back(const_reference __val) {
        if (_M_size + 1 >= _M_capacity) {
            reserve(_M_size + 1);
        }
        _M_alloc.construct(&_M_data[_M_size], __val);
        ++_M_size;
    }

    void push_back(value_type&& __val) {
        if (_M_size + 1 >= _M_capacity) {
            reserve(_M_size + 1);
        }
        _M_alloc.construct(&_M_data[_M_size], std::move(__val));
        ++_M_size;
    }

    template <class ..._Args>
    const_reference emplace_back(_Args &&...__args) {
        if (_M_size + 1 >= _M_capacity) {
            reserve(_M_size + 1);
        }
        pointer __p = &_M_data[_M_size];
        _M_alloc.construct(__p, std::forward<_Args>(__args)...);
        ++_M_size;
        return *__p;
    }

    void pop_back() noexcept {
        _M_size -= 1;
        std::destroy_at(&_M_data[_M_size]);
    }

    void resize(size_type __new_size) {
        if (__new_size < _M_size) {
            for (std::size_t __i = 0; __i < _M_size; ++__i) {
                std::destroy_at(&_M_data[__i]);
            }
        } else if (__new_size > _M_size) {
            reserve(__new_size);
            for ( std::size_t __i = _M_size; __i < __new_size; ++__i) {
                _M_alloc.construct(&_M_data[__i]);
            }
        }
        _M_size = __new_size;
    }

    void resize(size_type __new_size, const value_type& __x) {
        if (__new_size < _M_size) {
            for (std::size_t __i = 0; __i < _M_size; ++__i) {
                std::destroy_at(&_M_data[__i]);
            }
        } else if (__new_size > _M_size) {
            reserve(__new_size);
            for ( std::size_t __i = _M_size; __i < __new_size; ++__i) {
                _M_alloc.construct(&_M_data[__i], __x);
            }
        }
        _M_size = __new_size;
    }

    void swap(Vector &__other) noexcept {
        std::swap(_M_data, __other._M_data);
        std::swap(_M_size, __other._M_size);
        std::swap(_M_capacity, __other._M_capacity);
        std::swap(_M_alloc, __other._M_alloc);
    }
    
    // Iterators.
    iterator begin() noexcept {
        return _M_data;
    }

    const_iterator begin() const noexcept {
        return _M_data;
    }

    iterator end() noexcept {
        return _M_data + _M_size;
    }

    const_iterator end() const noexcept {
        return _M_data + _M_size;
    }

    const_iterator cbegin() const noexcept {
        return _M_data;
    }

    const_iterator cend() const noexcept {
        return _M_data + _M_size;
    }
    
    const_reverse_iterator crbegin() const noexcept {
        return std::make_reverse_iterator(_M_data + _M_size);
    }

    const_reverse_iterator crend() const noexcept {
        return std::make_reverse_iterator(_M_data);
    }

    reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(_M_data + _M_size);
    }

    reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(_M_data);
    }

    _LIBPENGCXX_DEFINE_COMPARISON(Vector);
};