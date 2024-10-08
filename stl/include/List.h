#pragma once
#include <cstddef> // size_t
#include <stdexcept> // std::out_of_range
#include <iterator> // std::reverse_iterator
#include <algorithm> // std::equal
#include <memory>
#include <initializer_list>
#include "_Common.h"

template <class _Tp>
struct ListBaseNode {
    ListBaseNode* _M_next;
    ListBaseNode* _M_prev;
    
    inline _Tp &value();
    inline const _Tp &value() const;
};

template <class _Tp>
struct ListValueNode : public ListBaseNode<_Tp> {
    union {
        _Tp _M_data;
    };
};

template <class _Tp>
inline _Tp &ListBaseNode<_Tp>::value() {
    return static_cast<ListValueNode<_Tp>*>(this)->_M_data;
}

template <class _Tp>
inline const _Tp &ListBaseNode<_Tp>::value() const {
    return static_cast<const ListValueNode<_Tp>*>(this)->_M_data;
}

template <class _Tp, class _Alloc = std::allocator<_Tp>>
struct List {
public:
    using value_type = _Tp;
    using allocator_type = _Alloc;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = _Tp *;
    using const_pointer = _Tp const *;
    using reference = _Tp &;
    using const_reference = _Tp const &;

private:
    using List_node = ListBaseNode<_Tp>;
    using List_value_node = ListValueNode<_Tp>;
    using List_node_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<List_value_node>;

    List_node _M_dummy;
    size_type _M_node_count;
    allocator_type _M_node_allocator;

    List_node* _M_create_node() {
        List_node_allocator alloc = _M_node_allocator;
        return alloc.allocate(1);
        // return std::allocator_traits<List_node_allocator>::allocate(alloc, 1);
    }
    void _M_destroy_node(List_node* __p) noexcept {
        List_node_allocator alloc = _M_node_allocator;
        alloc.deallocate(static_cast<List_value_node*>(__p), 1);
        // std::allocator_traits<List_node_allocator>::deallocate(alloc, static_cast<List_value_node*>(__p), 1);
    }

public:
    List() : _M_node_count(0), _M_node_allocator() {
        _M_dummy._M_next = &_M_dummy;
        _M_dummy._M_prev = &_M_dummy;
    }
    
    explicit List(const allocator_type& __a) : _M_node_count(0), _M_node_allocator(__a) {
        _M_dummy._M_next = &_M_dummy;
        _M_dummy._M_prev = &_M_dummy;
    }
    
    explicit List(size_type __n, const allocator_type& __a = allocator_type()) : _M_node_allocator(__a) {
        _uninit_assign(__n);
    }

    List(size_type __n, const value_type& __value, const allocator_type& __a = allocator_type()) : _M_node_allocator(__a) {
        _uninit_assign(__n, __value);
    }

    template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
	List(_InputIterator __first, _InputIterator __last, const allocator_type& __a = allocator_type()) : _M_node_allocator(__a) {
        _uninit_assign(__first, __last);
    }

    List(std::initializer_list<value_type> __l, const allocator_type& __a = allocator_type()) 
     : List(__l.begin(), __l.end(), __a) {}

    List(List const& __other) : _M_node_allocator(__other._M_node_allocator) {
        _uninit_assign(__other.cbegin(), __other.cend());
    }

    List &operator=(List const& __other) {
        assign(__other.cbegin(), __other.cend());
        return *this;
    }

    void assign(size_type __n, const value_type& __val) {
       clear();
       _uninit_assign(__n, __val);
    }

    template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
	void assign(_InputIterator __first, _InputIterator __last) {
        clear();
        _uninit_assign(__first, __last);
    }
    
    void assign(std::initializer_list<value_type> __l) {
       clear();
       _uninit_assign(__l.begin(), __l.end());
    }

    List &operator=(std::initializer_list<value_type> __l) {
       assign(__l);
    }

    List(List && __other) noexcept : _M_node_allocator(__other._M_node_allocator) {
       _uninit_move_assign(std::move(__other));
    }

    List &operator=(List && __other) noexcept {
        _M_node_allocator = std::move(__other._M_node_allocator);
        clear();
        _uninit_move_assign(std::move(__other));
        return *this;
    }

    ~List() {
        clear();
    }

    allocator_type get_allocator() const noexcept {
        return _M_node_allocator;
    }

public:
	template <class Visitor>
    void foreach(Visitor visit) {
        List_node* current = _M_dummy._M_next;
        while (current != &_M_dummy) {
            visit(current->value());
            current = current->_M_next;
        }
    }

private:
    void _uninit_assign(size_type n) {
        List_node *prev = &_M_dummy;
        _M_node_count = n;
        while (n) {
            List_node *node = _M_create_node();
            prev->_M_next = node;
            node->_M_prev = prev;
            _M_node_allocator.construct(&node->value());
            prev = node;
            --n;
        }
        _M_dummy._M_prev = prev;
        prev->_M_next = &_M_dummy;
    }

    void _uninit_assign(size_type n, const_reference val) {
        List_node *prev = &_M_dummy;
        _M_node_count = n;
        while (n) {
            List_node *node = _M_create_node();
            prev->_M_next = node;
            node->_M_prev = prev;
            _M_node_allocator.construct(&node->value(), val);
            prev = node;
            --n;
        }
        _M_dummy._M_prev = prev;
        prev->_M_next = &_M_dummy;
    }

    template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
    void _uninit_assign(_InputIterator first, _InputIterator last) {
        _M_node_count = 0;
        List_node *prev = &_M_dummy;
        while (first != last) {
            List_node *node = _M_create_node();
            prev->_M_next = node;
            node->_M_prev = prev;
            _M_node_allocator.construct(&node->value(), *first);
            prev = node;
            ++first;
            ++_M_node_count;
        }
        _M_dummy._M_prev = prev;
        prev->_M_next = &_M_dummy;
    }

    void _uninit_move_assign(List && __other) {
        auto prev = __other._M_dummy._M_prev;
        auto next = __other._M_dummy._M_next;
        prev->_M_next = &_M_dummy;
        next->_M_prev = &_M_dummy;
        _M_dummy = __other._M_dummy;
        __other._M_dummy._M_next = &__other._M_dummy;
        __other._M_dummy._M_prev = &__other._M_dummy;
        _M_node_count = __other._M_node_count;
        __other._M_node_count = 0;
    }

public:
    // Element access.
    reference front() noexcept {
        return _M_dummy._M_next->value();
    }

    const_reference front() const noexcept {
        return _M_dummy._M_next->value();
    }

    reference back() noexcept {
        return _M_dummy._M_prev->value();
    }

    const_reference back() const noexcept {
        return _M_dummy._M_prev->value();
    }

    // Capacity.
    bool empty() const noexcept {
        // return _M_dummy._M_prev == _M_dummy._M_next; 
        // 如果列表只有一个元素，_M_prev 和 _M_next 都会指向虚拟节点，导致empty方法错误地返回true。
        return _M_dummy._M_next == &_M_dummy;
        // return _M_node_count == 0; // 报错
    }

    size_type size() const noexcept {
        return _M_node_count;
    }

    static constexpr size_type max_size() noexcept {
	    return std::numeric_limits<size_type>::max();
    }

    // Iterators
    struct iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = _Tp;
        using difference_type = std::ptrdiff_t;
        using pointer = _Tp*;
        using reference = _Tp&;
    private:
        List_node* _M_node;
        friend List; // allow List to access the private members
    public:
        iterator() = default;
        explicit iterator(List_node* node) noexcept : _M_node(node) {}

        iterator& operator++() noexcept {
            _M_node = _M_node->_M_next;
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        iterator& operator--() noexcept {
            _M_node = _M_node->_M_prev;
            return *this;
        }

        iterator operator--(int) noexcept {
            iterator tmp = *this;
            --*this;
            return tmp;
        }

        reference operator*() const noexcept { 
            return _M_node->value(); 
        }

        pointer operator->() const noexcept { 
            return &_M_node->value(); 
        }

        bool operator!=(const iterator& other) const noexcept {
            return _M_node != other._M_node;
        }

        bool operator==(const iterator& other) const noexcept {
            return !(*this != other);
        }
    };

    struct const_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = _Tp;
        using difference_type = std::ptrdiff_t;
        using pointer = const _Tp*;
        using reference = const _Tp&;
    private:
        const List_node* _M_node;
        friend List;
    public:
        const_iterator() = default;
        explicit const_iterator(const List_node* node) noexcept : _M_node(node) {}
        const_iterator(const iterator& it) noexcept : _M_node(it._M_node) {}

        explicit operator iterator() noexcept {
            return iterator{const_cast<List_node*>(_M_node)};
        }

        const_iterator& operator++() noexcept {
            _M_node = _M_node->_M_next;
            return *this;
        }

        const_iterator operator++(int) noexcept {
            const_iterator tmp = *this;
            ++*this;
            return tmp;
        }

        const_iterator& operator--() noexcept {
            _M_node = _M_node->_M_prev;
            return *this;
        }

        const_iterator operator--(int) noexcept {
            const_iterator tmp = *this;
            --*this;
            return tmp;
        }

        reference operator*() const noexcept { 
            return _M_node->value(); 
        }

        pointer operator->() const noexcept { 
            return &_M_node->value(); 
        }

        bool operator!=(const const_iterator& other) const noexcept {
            return _M_node != other._M_node;
        }

        bool operator==(const const_iterator& other) const noexcept {
            return !(*this != other);
        }
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() noexcept {
        return iterator(_M_dummy._M_next);
    }

    const_iterator begin() const noexcept {
        return const_iterator(_M_dummy._M_next);
    }

    iterator end() noexcept {
        return iterator(&_M_dummy);
    }

    const_iterator end() const noexcept {
        return const_iterator(&_M_dummy);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(_M_dummy._M_next);
    }

    const_iterator cend() const noexcept {
        return const_iterator(&_M_dummy);
    }

    reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const noexcept {
        return std::make_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const noexcept {
        return std::make_reverse_iterator(cbegin());
    }
    
    // Modifiers
    void clear() noexcept {
        List_node* current = _M_dummy._M_next;
        while (current != &_M_dummy) {
            List_node* next = current->_M_next;
            _M_node_allocator.destroy(&current->value());
            _M_destroy_node(current);
            current = next;
        }
        _M_dummy._M_next = &_M_dummy;
        _M_dummy._M_prev = &_M_dummy;
        _M_node_count = 0;
    }

    iterator insert(const_iterator __position, const value_type& __x) {
        return emplace(__position, __x);
    }

    iterator insert(const_iterator __position, value_type&& __x) {
        return emplace(__position, std::move(__x));
    }

    iterator insert(const_iterator __position, size_type __n, const value_type& __x) {
        auto orig = __position;
        bool has_inserted = false;
        while (__n) {
            __position = emplace(__position, __x);
            if (!has_inserted) {
                has_inserted = true;
                orig = __position;
            }
            --__n;
            ++__position;
        }
        return iterator(orig);
    }

    template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
	iterator insert(const_iterator __position, _InputIterator __first, _InputIterator __last) {
        auto orig = __position;
        bool has_inserted = false;
        while (__first != __last) {
            __position = emplace(__position, *__first);
            if (!has_inserted) {
                has_inserted = true;
                orig = __position;
            }
            ++__position;
            ++__first;
        }
        return iterator(orig);
    }

    iterator insert(const_iterator __position, std::initializer_list<value_type> __l) {
        return insert(__position, __l.begin(), __l.end());
    }

    template <class ..._Args>
    iterator emplace(const_iterator __position, _Args&&... __args) {
        List_node *current = _M_create_node();
        List_node *next = const_cast<List_node*>(__position._M_node);
        List_node *prev = next->_M_prev;
        current->_M_prev = prev;
        prev->_M_next = current;
        current->_M_next = next;
        next->_M_prev = current;
        _M_node_allocator.construct(&current->value(), std::forward<_Args>(__args)...);
        ++_M_node_count;
        return iterator(current);
    }

    iterator erase(const_iterator __pos) noexcept {
        List_node *node =  const_cast<List_node*>(__pos._M_node);
        List_node *prev = node->_M_prev;
        List_node *next = node->_M_next;
        prev->_M_next = next;
        next->_M_prev = prev;
        _M_node_allocator.destroy(&node->value());
        _M_destroy_node(node);
        --_M_node_count;
        return iterator(next);
    }

    iterator erase(const_iterator __first, const_iterator __last) {
        while (__first != __last) {
            __first = erase(__first);
        }
        return iterator(__first);
    }

    void push_back(const_reference __val) {
        emplace_back(__val);
    }

    void push_back(value_type&& __val) {
        emplace_back(std::move(__val));
    }

    template <class ..._Args>
    const_reference emplace_back(_Args &&...__args) {
        List_node *node = _M_create_node();
        List_node *prev = _M_dummy._M_prev;
        prev->_M_next = node;
        node->_M_prev = prev;
        node->_M_next = &_M_dummy;
        _M_node_allocator.construct(&node->value(), std::forward<_Args>(__args)...);
        _M_dummy._M_prev = node;
        ++_M_node_count;
        return node->value();
    }

    void pop_back() noexcept {
        erase(std::prev(end()));
    }

    void push_front(const_reference __val) {
        emplace_front(__val);
    }

    void push_front(value_type&& __val) {
        emplace_front(std::move(__val));
    }

    template <class ..._Args>
    const_reference emplace_front(_Args&&... __args) {
        List_node *node = _M_create_node();
        List_node *next = _M_dummy._M_next;
        next->_M_prev = node;
        node->_M_next = next;
        node->_M_prev = &_M_dummy;
        _M_node_allocator.construct(&node->value(), std::forward<_Args>(__args)...);
        _M_dummy._M_next = node;
        ++_M_node_count;
        return node->value();
    }

    void pop_front() noexcept {
        erase(begin());
    }

    void resize(size_type __new_size) {
        resize(__new_size, value_type());
    }

    void resize(size_type __new_size, const value_type& __x) {
        while (_M_node_count < __new_size) {
            emplace_back(__x);
        }
        while (_M_node_count > __new_size) {
            pop_back();
        }
    }

    void swap(List &__other) noexcept {
        std::swap(_M_dummy, __other._M_dummy);
        std::swap(_M_node_count, __other._M_node_count);
        std::swap(_M_node_allocator, __other._M_node_allocator);

        // Update the links of the dummy nodes to point to the correct dummy nodes
        if (_M_dummy._M_next) {
            _M_dummy._M_next->_M_prev = &_M_dummy;
        }
        if (_M_dummy._M_prev) {
            _M_dummy._M_prev->_M_next = &_M_dummy;
        }

        if (__other._M_dummy._M_next) {
            __other._M_dummy._M_next->_M_prev = &__other._M_dummy;
        }
        if (__other._M_dummy._M_prev) {
            __other._M_dummy._M_prev->_M_next = &__other._M_dummy;
        }
    }

    // Operations.
    void merge(List&& __x) {
        merge(std::move(__x), std::less<>());
    }

    void merge(List& __x) {
        merge(std::move(__x));
    }

    template<typename _StrictWeakOrdering>
	void merge(List&& __x, _StrictWeakOrdering __comp) {
        if (this == &__x) return;

        iterator __first1 = begin();
        iterator __last1 = end();
        iterator __first2 = __x.begin();
        iterator __last2 = __x.end();

        while (__first1 != __last1 && __first2 != __last2) {
            if (__comp(*__first2, *__first1)) {
                iterator __next = __first2;
                ++__next;
                splice(__first1, std::move(__x), __first2);
                __first2 = __next;
            } else {
                ++__first1;
            }
        }
        if (__first2 != __last2) {
            splice(__last1, std::move(__x), __first2, __last2);
        }
    }
    

    template<typename _StrictWeakOrdering>
	void merge(List& __x, _StrictWeakOrdering __comp) {
        merge(std::move(__x), __comp);
    }

    void splice(const_iterator __position, List&& __x) noexcept {
        if (__x.empty()) return;
        // method 1
        // insert(__position, std::make_move_iterator(__x.begin()), std::make_move_iterator(__x.end()));
        // __x.clear(); 
        // method 2
        // splice(__position, std::move(__x), std::move(__x).begin(), std::move(__x).end());

        List_node* first = __x._M_dummy._M_next;
        List_node* last = __x._M_dummy._M_prev;

        // Adjust the links in the source list
        __x._M_dummy._M_next = &__x._M_dummy;
        __x._M_dummy._M_prev = &__x._M_dummy;

        // Adjust the links in the destination list
        List_node* pos = const_cast<List_node*>(__position._M_node);
        List_node* pos_prev = pos->_M_prev;

        pos_prev->_M_next = first;
        first->_M_prev = pos_prev;
        last->_M_next = pos;
        pos->_M_prev = last;

        _M_node_count += __x._M_node_count;
        __x._M_node_count = 0;
    }

    void splice(const_iterator __position, List& __x) noexcept { 
        splice(__position, std::move(__x)); 
    }

    void splice(const_iterator __position, List&& __x, const_iterator __i) noexcept {
        if (__x.empty() || __i == __x.end()) return;

        List_node* node = const_cast<List_node*>(__i._M_node);
        List_node* prev = node->_M_prev;
        List_node* next = node->_M_next;

        if (prev) prev->_M_next = next;
        if (next) next->_M_prev = prev;

        List_node* pos = const_cast<List_node*>(__position._M_node);
        List_node* pos_prev = pos->_M_prev;

        if (pos_prev) pos_prev->_M_next = node;
        node->_M_prev = pos_prev;
        node->_M_next = pos;
        pos->_M_prev = node;

        --__x._M_node_count;
        ++_M_node_count;
    }

    void splice(const_iterator __position, List& __x, const_iterator __i) noexcept { 
        splice(__position, std::move(__x), __i); 
    }

    void splice(const_iterator __position, List&& __x, const_iterator __first,
	     const_iterator __last) noexcept {
        if (__first == __last) return;

        size_type count = std::distance(__first, __last);
        __x._M_node_count -= count;
        _M_node_count += count;

        if (__last != __x.begin() && !__x.empty()) {
            --__last; // 闭开区间 [first, last)
        }
        List_node* first_node = const_cast<List_node*>(__first._M_node);
        List_node* last_node = const_cast<List_node*>(__last._M_node);
        
        List_node* prev = first_node->_M_prev;
        List_node* next = last_node->_M_next;
        
        if (prev) prev->_M_next = next;
        if (next) next->_M_prev = prev;

        List_node* pos = const_cast<List_node*>(__position._M_node);
        List_node* pos_prev = pos->_M_prev;

        if (pos_prev) pos_prev->_M_next = first_node;

        first_node->_M_prev = pos_prev;
        last_node->_M_next = pos;
        pos->_M_prev = last_node;
    }

    void splice(const_iterator __position, List& __x, const_iterator __first,
	     const_iterator __last) noexcept {
        splice(__position, std::move(__x), __first, __last); 
    }

    void remove(const_reference __value) noexcept {
        auto __first = begin();
        auto __last = end();
        while (__first != __last) {
            if (*__first == __value) {
                __first = erase(__first);
            } else {
                ++__first;
            }
        }
    }

    template<typename _Predicate>
	void remove_if(_Predicate &&__pred) noexcept {
        auto __first = begin();
        auto __last = end();
        while (__first != __last) {
            if (__pred(*__first)) {
                __first = erase(__first);
            } else {
                ++__first;
            }
        }
    }

    void reverse() {
        if (_M_node_count <= 1) return;

        List_node* current = _M_dummy._M_next;
        while (current != &_M_dummy) {
            std::swap(current->_M_next, current->_M_prev);
            current = current->_M_prev;
        }
        std::swap(_M_dummy._M_next, _M_dummy._M_prev);
    }

    void unique() {
        unique(std::equal_to<>());
    }

    template<typename _BinaryPredicate>
    void unique(_BinaryPredicate __binary_pred) {
        iterator first = begin();
        iterator last = end();
        if (first == last) return;

        iterator next = first;
        while (++next != last) {
            if (__binary_pred(*first, *next)) {
                erase(next);
            } else {
                first = next;
            }
            next = first;
        }
    }

    void sort() {
        sort(std::less<>());
    }


    template<typename _StrictWeakOrdering>
    void sort(_StrictWeakOrdering __comp) {
        
    }

    _LIBPENGCXX_DEFINE_COMPARISON(List);
};