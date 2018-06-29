#ifndef CIRCULAR_BUFFER_CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_CIRCULAR_BUFFER_H

#include <cstdlib>
#include <iterator>
#include <memory>
#include <cassert>
#include <utility>
#include <type_traits>

template <typename T>
struct circular_buffer {
private:

    template <typename S>
    struct basic_iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = S;
        using pointer = S*;
        using reference = S&;

        basic_iterator(size_t ind, size_t cap, T* data, size_t ind_begin)
                : ind(ind)
                , cap(cap)
                , data(data)
                , ind_begin(ind_begin)
        {}

        basic_iterator(const basic_iterator & other)
                : ind(other.ind)
                , cap(other.cap)
                , data(other.data)
                , ind_begin(other.ind_begin)
        {}

        template <typename U>
        basic_iterator(basic_iterator<U> const& other, typename std::enable_if<std::is_same<U const, S>::value && std::is_const<S>::value>::type* = nullptr)
                : ind(other.ind)
                , cap(other.cap)
                , data(other.data)
                , ind_begin(other.ind_begin)
        {}

        ~basic_iterator() = default;

        basic_iterator& operator=(const basic_iterator & other) {
            basic_iterator tmp(other);
            swap(tmp);
            return *this;
        }

        void swap(basic_iterator & other) {
            std::swap(ind, other.ind);
            std::swap(cap, other.cap);
            std::swap(data, other.data);
            std::swap(ind_begin, other.ind_begin);
        }

        reference operator*() const {
            return data[ind];
        };
        pointer operator->() const {
            return &data[ind];
        }

        basic_iterator& operator++() {
            ind = (ind + 1) % cap;
            return *this;
        }
        basic_iterator operator++(int) {
            auto old = *this;
            ind = (ind + 1) % cap;
            return old;
        }

        basic_iterator& operator--()  {
            ind = (ind - 1 + cap) % cap;
            return *this;
        }
        basic_iterator operator--(int) {
            auto old = *this;
            ind = (ind - 1 + cap) % cap;
            return old;
        }

        friend bool operator==(basic_iterator const& a, basic_iterator const& b) {
            return (a.ind == b.ind && a.data == b.data);
        }

        friend bool operator!=(basic_iterator const& a, basic_iterator const& b) {
            return !(a == b);
        }

        friend bool operator<(basic_iterator const& a, basic_iterator const& b) {
            return a.get_pos() < b.get_pos();
        }
        friend bool operator<=(basic_iterator const& a, basic_iterator const& b) {
            return (a < b || a == b);
        }
        friend bool operator>(basic_iterator const& a, basic_iterator const& b) {
            return !(a <= b);
        }
        friend bool operator>=(basic_iterator const& a, basic_iterator const& b) {
            return !(a < b);
        }
        basic_iterator operator+(size_t ch) const {
            size_t new_ind = (ind + ch) % cap;
            return basic_iterator(new_ind, cap, data, ind_begin);
        }
        basic_iterator& operator+=(size_t ch) {
            ind = (ind + ch) % cap;
            return *this;
        }
        basic_iterator operator-(size_t ch) const {
            size_t new_ind = (ind - ch + cap) % cap;
            return basic_iterator(new_ind, cap, data, ind_begin);
        }
        basic_iterator& operator-=(size_t ch) {
            ind = (ind - ch + cap) % cap;
            return *this;
        }

        friend difference_type operator-(const basic_iterator& a, const basic_iterator& b) {
            return a.get_pos() - b.get_pos();
        }

        size_t get_pos() const {
            return (ind + cap - ind_begin) % cap;
        }

    private:
        size_t ind;
        size_t cap;
        T* data;
        size_t ind_begin;
        friend struct circular_buffer;
    };

public:
    using iterator = basic_iterator<T>;
    using const_iterator = basic_iterator<T const>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    circular_buffer();
    circular_buffer(circular_buffer const& other);
    circular_buffer& operator=(circular_buffer const& other);
    ~circular_buffer();

    size_t size() const;
    bool empty() const;
    void clear();
    T& back() const;
    T& front() const;
    void push_back(T const& value);
    void push_front(T const& value);
    void pop_back();
    void pop_front();
    T& operator[](size_t ind) const;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    iterator insert(const_iterator pos, T const& value);
    iterator erase(const_iterator pos);

    void swap(circular_buffer& oth);

private:
    void reserve(size_t n);

    size_t sz;
    size_t cap;
    T* data;
    size_t ind_begin;
    size_t ind_end;
};

template<typename T>
circular_buffer<T>::circular_buffer()
        : sz(0)
        , cap(0)
        , data(nullptr)
        , ind_begin(0)
        , ind_end(0)
{}

template<typename T>
circular_buffer<T>::circular_buffer(circular_buffer const &oth) : circular_buffer() {
    for (size_t i = 0; i < oth.size(); i++) {
        push_back(oth[i]);
    }
}

template<typename T>
circular_buffer<T> &circular_buffer<T>::operator=(circular_buffer const & oth) {
    circular_buffer tmp(oth);
    swap(tmp);
    return *this;
}


template<typename T>
circular_buffer<T>::~circular_buffer() {
    clear();
}

template<typename T>
size_t circular_buffer<T>::size() const {
    return sz;
}

template<typename T>
bool circular_buffer<T>::empty() const {
    return sz == 0;
}

template<typename T>
T & circular_buffer<T>::back() const {
    assert(sz > 0);
    return data[(ind_end - 1 + cap) % cap];
}

template<typename T>
T& circular_buffer<T>::front() const {
    assert(sz > 0);
    return data[ind_begin];
}

template<typename T>
void circular_buffer<T>::reserve(size_t n) {
    if (sz >= n) {
        return;
    }
    circular_buffer tmp;
    tmp.cap = n;
    tmp.data = static_cast<T*>(operator new(n * sizeof(T)));
    for (size_t i = 0; i < sz; i++) {
        new (tmp.data + tmp.ind_end) T(data[(ind_begin + i) % cap]);
        tmp.ind_end++;
        tmp.sz++;
    }
    swap(tmp);
}

template<typename T>
void circular_buffer<T>::clear() {
    while (sz > 0) {
        pop_back();
    }
    operator delete(data);
    sz = cap = ind_begin = ind_end = 0;
    data = nullptr;
}

template<typename T>
void circular_buffer<T>::push_back(T const& value) {
    if (empty()) {
        reserve(4);
    } else if (sz == cap - 1) {
        reserve(cap * 2);
    }
    new (data + ind_end) T(value);
    ind_end = (ind_end + 1) % cap;
    sz++;
}

template<typename T>
void circular_buffer<T>::push_front(const T &value) {
    if (empty()) {
        reserve(4);
    } else if (sz == cap - 1) {
        reserve(sz * 2);
    }
    new (data + (ind_begin - 1 + cap) % cap) T(value);
    ind_begin = (ind_begin - 1 + cap) % cap;
    sz++;
}

template<typename T>
T& circular_buffer<T>::operator[](size_t ind) const {
    assert(ind < sz);
    return data[(ind_begin + ind) % cap];
}

template<typename T>
void circular_buffer<T>::pop_back() {
    assert(sz > 0);
    sz--;
    ind_end = (ind_end - 1 + cap) % cap;
    data[ind_end].~T();
}

template<typename T>
void circular_buffer<T>::pop_front() {
    assert(sz > 0);
    sz--;
    data[ind_begin].~T();
    ind_begin = (ind_begin + 1) % cap;
}

template<typename T>
typename circular_buffer<T>::iterator circular_buffer<T>::begin() {
    return circular_buffer<T>::iterator(ind_begin, cap, data, ind_begin);
}

template<typename T>
typename circular_buffer<T>::const_iterator circular_buffer<T>::begin() const {
    return circular_buffer<T>::const_iterator(ind_begin, cap, data, ind_begin);
}

template<typename T>
typename circular_buffer<T>::iterator circular_buffer<T>::end() {
    return circular_buffer<T>::iterator(ind_end, cap, data, ind_begin);
}

template<typename T>
typename circular_buffer<T>::const_iterator circular_buffer<T>::end() const {
    return circular_buffer<T>::const_iterator(ind_end, cap, data, ind_begin);
}

template<typename T>
typename circular_buffer<T>::reverse_iterator circular_buffer<T>::rbegin() {
    return std::reverse_iterator<iterator>(end());
}

template<typename T>
typename circular_buffer<T>::const_reverse_iterator circular_buffer<T>::rbegin() const {
    return std::reverse_iterator<const_iterator>(end());
}

template<typename T>
typename circular_buffer<T>::reverse_iterator circular_buffer<T>::rend() {
    return std::reverse_iterator<iterator>(begin());
}

template<typename T>
typename circular_buffer<T>::const_reverse_iterator circular_buffer<T>::rend() const{
    return std::reverse_iterator<const_iterator>(begin());
}

template<typename T>
void circular_buffer<T>::swap(circular_buffer<T> &oth) {
    std::swap(sz, oth.sz);
    std::swap(cap, oth.cap);
    std::swap(data, oth.data);
    std::swap(ind_begin, oth.ind_begin);
    std::swap(ind_end, oth.ind_end);
}

template<typename T>
typename circular_buffer<T>::iterator circular_buffer<T>::insert(circular_buffer<T>::const_iterator pos, T const& value) {
    if (pos == begin()) {
        push_front(value);
        return begin();
    }
    if (pos == end()) {
        push_back(value);
        return --end();
    }
    if (pos.get_pos() <= sz / 2) {
        push_front(value);
        auto it1 = begin();
        auto it2 = ++begin();
        while (it2 != pos) {
            auto tmp = *it1;
            *it1 = *it2;
            *it2 = tmp;
            it1++;
            it2++;
        }
        return it1;
    } else {
        push_back(value);
        auto it1 = --(--end());
        auto it2 = --end();
        while (it2 != pos) {
            auto tmp = *it1;
            *it1 = *it2;
            *it2 = tmp;
            it1--;
            it2--;
        }
        return it2;
    }
}

template<typename T>
typename circular_buffer<T>::iterator circular_buffer<T>::erase(circular_buffer<T>::const_iterator pos) {
    assert(pos.get_pos() < sz);
    if (pos == begin()) {
        pop_front();
        return begin();
    }
    if (pos == --end()) {
        pop_back();
        return --end();
    }
    size_t ind_pos_relative = pos.get_pos();
    size_t ind_pos_mem = (ind_pos_relative + ind_begin) % cap;
    if (ind_pos_relative <= sz / 2) {
        iterator it2(ind_pos_mem, cap, data, ind_begin);
        while (it2 != begin()) {
            auto it1 = it2 - 1;
            auto tmp = *it1;
            *it1 = *it2;
            *it2 = tmp;
            it2--;
        }
        pop_front();
        return iterator(ind_pos_mem, cap, data, ind_begin) + 1;
    } else {
        iterator it1(ind_pos_mem, cap, data, ind_begin);
        auto it2 = it1 + 1;
        while (it2 != end()) {
            auto tmp = *it1;
            *it1 = *it2;
            *it2 = tmp;
            it1++;
            it2++;
        }
        pop_back();
        return iterator(ind_pos_mem, cap, data, ind_begin);
    }
}

template <typename T>
void swap(circular_buffer<T>& a, circular_buffer<T>& b) {
    a.swap(b);
}

#endif