// Copyright 2025 Bryan Wong
// Adapted from GCC testsuite

// -*- C++ -*-
// Iterator Wrappers for the C++ library testsuite.
//
// Copyright (C) 2004-2025 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.
//

// This file provides the following:
//
// input_iterator_wrapper, output_iterator_wrapper
// forward_iterator_wrapper, bidirectional_iterator_wrapper and
// random_access_wrapper, which attempt to exactly perform the requirements
// of these types of iterators. These are constructed from the class
// test_container, which is given two pointers to T and an iterator type.

#include "rxx/config.h"

#include <type_traits>

#define ITERATOR_VERIFY(x)

RXX_DEFAULT_NAMESPACE_BEGIN
namespace tests {
/**
 * @brief Simple container for holding two pointers.
 *
 * Note that input_iterator_wrapper changes first to denote
 * how the valid range of == , ++, etc. change as the iterators are used.
 */
template <typename T>
struct BoundsContainer {
    T* first;
    T* last;

    BoundsContainer(T* first, T* last) : first(first), last(last) {}

    size_t size() const { return last - first; }
};

// Simple container for holding state of a set of output iterators.
template <typename T>
struct OutputContainer : public BoundsContainer<T> {
    T* incrementedto;
    bool* writtento;

    OutputContainer(T* first, T* last)
        : BoundsContainer<T>(first, last)
        , incrementedto(first)
        , writtento(new bool[this->size()]()) {}

    ~OutputContainer() { delete[] writtento; }
};

// Produced by output_iterator to allow limited writing to pointer
template <typename T>
class WritableObject {
    T* ptr;

public:
    OutputContainer<T>* SharedInfo;

    WritableObject(T* ptr, OutputContainer<T>* info)
        : ptr(ptr)
        , SharedInfo(info) {}

    template <typename U>
    typename std::enable_if<std::is_assignable<T&, U>::value>::type operator=(
        U&& new_val) const {
        ITERATOR_VERIFY(SharedInfo->writtento[ptr - SharedInfo->first] == 0);
        SharedInfo->writtento[ptr - SharedInfo->first] = 1;
        *ptr = std::forward<U>(new_val);
    }
};

/**
 * @brief output_iterator wrapper for pointer
 *
 * This class takes a pointer and wraps it to provide exactly
 * the requirements of a output_iterator. It should not be
 * instantiated directly, but generated from a test_container
 */
template <class T>
struct output_iterator_wrapper {
protected:
    output_iterator_wrapper() : ptr(0), SharedInfo(0) {}

public:
    typedef std::output_iterator_tag iterator_category;
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    typedef OutputContainer<T> ContainerType;
    T* ptr;
    ContainerType* SharedInfo;

    output_iterator_wrapper(T* ptr, ContainerType* info)
        : ptr(ptr)
        , SharedInfo(info) {
        ITERATOR_VERIFY(ptr >= SharedInfo->first && ptr <= SharedInfo->last);
    }

    output_iterator_wrapper(output_iterator_wrapper const&) = default;

    output_iterator_wrapper& operator=(
        output_iterator_wrapper const&) = default;

    WritableObject<T> operator*() const {
        ITERATOR_VERIFY(ptr < SharedInfo->last);
        ITERATOR_VERIFY(
            SharedInfo->writtento[ptr - SharedInfo->first] == false);
        return WritableObject<T>(ptr, SharedInfo);
    }

    output_iterator_wrapper& operator++() {
        ITERATOR_VERIFY(SharedInfo && ptr < SharedInfo->last);
        ITERATOR_VERIFY(ptr >= SharedInfo->incrementedto);
        ptr++;
        SharedInfo->incrementedto = ptr;
        return *this;
    }

    output_iterator_wrapper operator++(int) {
        output_iterator_wrapper<T> tmp = *this;
        ++*this;
        return tmp;
    }

    template <typename U>
    void operator,(U const&) const = delete;

    void operator&() const = delete;
};

template <typename T, typename U>
void operator,(T const&, output_iterator_wrapper<U> const&) = delete;

/**
 * @brief input_iterator wrapper for pointer
 *
 * This class takes a pointer and wraps it to provide exactly
 * the requirements of a input_iterator. It should not be
 * instantiated directly, but generated from a test_container
 */
template <class T>
class input_iterator_wrapper {
    struct post_inc_proxy {
        struct deref_proxy {
            T* ptr;
            operator T const&() const { return *ptr; }
        } p;

        deref_proxy operator*() const { return p; }
    };

protected:
    input_iterator_wrapper() : ptr(0), SharedInfo(0) {}

public:
    typedef std::input_iterator_tag iterator_category;
    typedef std::remove_cv_t<T> value_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    typedef BoundsContainer<T> ContainerType;
    T* ptr;
    ContainerType* SharedInfo;

    input_iterator_wrapper(T* ptr, ContainerType* info)
        : ptr(ptr)
        , SharedInfo(info) {
        ITERATOR_VERIFY(ptr >= SharedInfo->first && ptr <= SharedInfo->last);
    }

    input_iterator_wrapper(input_iterator_wrapper const&) = default;

    input_iterator_wrapper& operator=(input_iterator_wrapper const&) = default;

    bool operator==(input_iterator_wrapper const& in) const {
        ITERATOR_VERIFY(SharedInfo && SharedInfo == in.SharedInfo);
        ITERATOR_VERIFY(
            ptr >= SharedInfo->first && in.ptr >= SharedInfo->first);
        return ptr == in.ptr;
    }

    bool operator!=(input_iterator_wrapper const& in) const {
        return !(*this == in);
    }

    T& operator*() const {
        ITERATOR_VERIFY(SharedInfo && ptr < SharedInfo->last);
        ITERATOR_VERIFY(ptr >= SharedInfo->first);
        return *ptr;
    }

    T* operator->() const { return &**this; }

    input_iterator_wrapper& operator++() {
        ITERATOR_VERIFY(SharedInfo && ptr < SharedInfo->last);
        ITERATOR_VERIFY(ptr >= SharedInfo->first);
        ptr++;
        SharedInfo->first = ptr;
        return *this;
    }

    post_inc_proxy operator++(int) {
        post_inc_proxy tmp = {{ptr}};
        ++*this;
        return tmp;
    }

    template <typename U>
    void operator,(U const&) const = delete;

    void operator&() const = delete;
};

template <typename T, typename U>
void operator,(T const&, input_iterator_wrapper<U> const&) = delete;

/**
 * @brief forward_iterator wrapper for pointer
 *
 * This class takes a pointer and wraps it to provide exactly
 * the requirements of a forward_iterator. It should not be
 * instantiated directly, but generated from a test_container
 */
template <class T>
struct forward_iterator_wrapper : public input_iterator_wrapper<T> {
    typedef BoundsContainer<T> ContainerType;
    typedef std::forward_iterator_tag iterator_category;

    forward_iterator_wrapper(T* _ptr, ContainerType* SharedInfo_in)
        : input_iterator_wrapper<T>(_ptr, SharedInfo_in) {}

    forward_iterator_wrapper() {}

    forward_iterator_wrapper(forward_iterator_wrapper const&) = default;

    forward_iterator_wrapper& operator=(
        forward_iterator_wrapper const&) = default;

    T& operator*() const {
        ITERATOR_VERIFY(this->SharedInfo && this->ptr < this->SharedInfo->last);
        return *(this->ptr);
    }

    T* operator->() const { return &**this; }

    forward_iterator_wrapper& operator++() {
        ITERATOR_VERIFY(this->SharedInfo && this->ptr < this->SharedInfo->last);
        this->ptr++;
        return *this;
    }

    forward_iterator_wrapper operator++(int) {
        forward_iterator_wrapper<T> tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(forward_iterator_wrapper const& it) const noexcept {
        // Since C++14 value-initialized forward iterators are comparable.
        if (this->SharedInfo == nullptr || it.SharedInfo == nullptr)
            return this->SharedInfo == it.SharedInfo && this->ptr == it.ptr;

        input_iterator_wrapper<T> const& base_this = *this;
        input_iterator_wrapper<T> const& base_that = it;
        return base_this == base_that;
    }

    bool operator!=(forward_iterator_wrapper const& it) const noexcept {
        return !(*this == it);
    }
};

/**
 * @brief bidirectional_iterator wrapper for pointer
 *
 * This class takes a pointer and wraps it to provide exactly
 * the requirements of a bidirectional_iterator. It should not be
 * instantiated directly, but generated from a test_container
 */
template <class T>
struct bidirectional_iterator_wrapper : public forward_iterator_wrapper<T> {
    typedef BoundsContainer<T> ContainerType;
    typedef std::bidirectional_iterator_tag iterator_category;

    bidirectional_iterator_wrapper(T* _ptr, ContainerType* SharedInfo_in)
        : forward_iterator_wrapper<T>(_ptr, SharedInfo_in) {}

    bidirectional_iterator_wrapper() : forward_iterator_wrapper<T>() {}

    bidirectional_iterator_wrapper(
        bidirectional_iterator_wrapper const&) = default;

    bidirectional_iterator_wrapper& operator=(
        bidirectional_iterator_wrapper const&) = default;

    bidirectional_iterator_wrapper& operator++() {
        ITERATOR_VERIFY(this->SharedInfo && this->ptr < this->SharedInfo->last);
        this->ptr++;
        return *this;
    }

    bidirectional_iterator_wrapper operator++(int) {
        bidirectional_iterator_wrapper<T> tmp = *this;
        ++*this;
        return tmp;
    }

    bidirectional_iterator_wrapper& operator--() {
        ITERATOR_VERIFY(
            this->SharedInfo && this->ptr > this->SharedInfo->first);
        this->ptr--;
        return *this;
    }

    bidirectional_iterator_wrapper operator--(int) {
        bidirectional_iterator_wrapper<T> tmp = *this;
        --*this;
        return tmp;
    }
};

/**
 * @brief random_access_iterator wrapper for pointer
 *
 * This class takes a pointer and wraps it to provide exactly
 * the requirements of a random_access_iterator. It should not be
 * instantiated directly, but generated from a test_container
 */
template <class T>
struct random_access_iterator_wrapper :
    public bidirectional_iterator_wrapper<T> {
    typedef BoundsContainer<T> ContainerType;
    typedef std::random_access_iterator_tag iterator_category;

    random_access_iterator_wrapper(T* _ptr, ContainerType* SharedInfo_in)
        : bidirectional_iterator_wrapper<T>(_ptr, SharedInfo_in) {}

    random_access_iterator_wrapper() : bidirectional_iterator_wrapper<T>() {}

    random_access_iterator_wrapper(
        random_access_iterator_wrapper const&) = default;

    random_access_iterator_wrapper& operator=(
        random_access_iterator_wrapper const&) = default;

    random_access_iterator_wrapper& operator++() {
        ITERATOR_VERIFY(this->SharedInfo && this->ptr < this->SharedInfo->last);
        this->ptr++;
        return *this;
    }

    random_access_iterator_wrapper operator++(int) {
        random_access_iterator_wrapper<T> tmp = *this;
        ++*this;
        return tmp;
    }

    random_access_iterator_wrapper& operator--() {
        ITERATOR_VERIFY(
            this->SharedInfo && this->ptr > this->SharedInfo->first);
        this->ptr--;
        return *this;
    }

    random_access_iterator_wrapper operator--(int) {
        random_access_iterator_wrapper<T> tmp = *this;
        --*this;
        return tmp;
    }

    random_access_iterator_wrapper& operator+=(ptrdiff_t n) {
        if (n > 0) {
            ITERATOR_VERIFY(n <= this->SharedInfo->last - this->ptr);
            this->ptr += n;
        } else {
            ITERATOR_VERIFY(-n <= this->ptr - this->SharedInfo->first);
            this->ptr += n;
        }
        return *this;
    }

    random_access_iterator_wrapper& operator-=(ptrdiff_t n) {
        return *this += -n;
    }

    random_access_iterator_wrapper operator-(ptrdiff_t n) const {
        random_access_iterator_wrapper<T> tmp = *this;
        return tmp -= n;
    }

    ptrdiff_t operator-(random_access_iterator_wrapper<T> const& in) const {
        ITERATOR_VERIFY(this->SharedInfo == in.SharedInfo);
        return this->ptr - in.ptr;
    }

    T& operator[](ptrdiff_t n) const { return *(*this + n); }

    bool operator<(random_access_iterator_wrapper<T> const& in) const {
        ITERATOR_VERIFY(this->SharedInfo == in.SharedInfo);
        return this->ptr < in.ptr;
    }

    bool operator>(random_access_iterator_wrapper<T> const& in) const {
        return in < *this;
    }

    bool operator>=(random_access_iterator_wrapper<T> const& in) const {
        return !(*this < in);
    }

    bool operator<=(random_access_iterator_wrapper<T> const& in) const {
        return !(*this > in);
    }
};

template <typename T>
random_access_iterator_wrapper<T> operator+(
    random_access_iterator_wrapper<T> it, ptrdiff_t n) {
    return it += n;
}

template <typename T>
random_access_iterator_wrapper<T> operator+(
    ptrdiff_t n, random_access_iterator_wrapper<T> it) {
    return it += n;
}

/**
 * @brief A container-type class for holding iterator wrappers
 * test_container takes two parameters, a class T and an iterator
 * wrapper templated by T (for example forward_iterator_wrapper<T>.
 * It takes two pointers representing a range and presents them as
 * a container of iterators.
 */
template <class T, template <class TT> class ItType>
struct test_container {
    typename ItType<T>::ContainerType bounds;

    test_container(T* _first, T* _last) : bounds(_first, _last) {}

    template <size_t N>
    explicit test_container(T (&arr)[N]) : bounds(arr, arr + N) {}

    ItType<T> it(int pos) {
        ITERATOR_VERIFY(pos >= 0 && (unsigned)pos <= size());
        return ItType<T>(bounds.first + pos, &bounds);
    }

    ItType<T> it(T* pos) {
        ITERATOR_VERIFY(pos >= bounds.first && pos <= bounds.last);
        return ItType<T>(pos, &bounds);
    }

    T const& val(int pos) { return (bounds.first)[pos]; }

    ItType<T> begin() { return it(bounds.first); }

    ItType<T> end() { return it(bounds.last); }

    size_t size() const { return bounds.size(); }
};

template <typename T>
using output_container = test_container<T, output_iterator_wrapper>;

template <typename T>
using input_container = test_container<T, input_iterator_wrapper>;

template <typename T>
using forward_container = test_container<T, forward_iterator_wrapper>;

template <typename T>
using bidirectional_container =
    test_container<T, bidirectional_iterator_wrapper>;

template <typename T>
using random_access_container =
    test_container<T, random_access_iterator_wrapper>;

template <typename T>
struct contiguous_iterator_wrapper : random_access_iterator_wrapper<T> {
    using random_access_iterator_wrapper<T>::random_access_iterator_wrapper;

    using iterator_concept = std::contiguous_iterator_tag;

    // Use an integer-class type to try and break the library code.
    using difference_type = ptrdiff_t;

    contiguous_iterator_wrapper& operator++() {
        random_access_iterator_wrapper<T>::operator++();
        return *this;
    }

    contiguous_iterator_wrapper& operator--() {
        random_access_iterator_wrapper<T>::operator--();
        return *this;
    }

    contiguous_iterator_wrapper operator++(int) {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    contiguous_iterator_wrapper operator--(int) {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    contiguous_iterator_wrapper& operator+=(difference_type n) {
        auto d = static_cast<ptrdiff_t>(n);
        random_access_iterator_wrapper<T>::operator+=(d);
        return *this;
    }

    friend contiguous_iterator_wrapper operator+(
        contiguous_iterator_wrapper iter, difference_type n) {
        return iter += n;
    }

    friend contiguous_iterator_wrapper operator+(
        difference_type n, contiguous_iterator_wrapper iter) {
        return iter += n;
    }

    contiguous_iterator_wrapper& operator-=(difference_type n) {
        return *this += -n;
    }

    friend contiguous_iterator_wrapper operator-(
        contiguous_iterator_wrapper iter, difference_type n) {
        return iter -= n;
    }

    friend difference_type operator-(
        contiguous_iterator_wrapper l, contiguous_iterator_wrapper r) {
        random_access_iterator_wrapper<T> const& lbase = l;
        random_access_iterator_wrapper<T> const& rbase = r;
        return static_cast<difference_type>(lbase - rbase);
    }

    decltype(auto) operator[](difference_type n) const {
        auto d = static_cast<ptrdiff_t>(n);
        return random_access_iterator_wrapper<T>::operator[](d);
    }
};

template <typename T>
using contiguous_container = test_container<T, contiguous_iterator_wrapper>;

// A move-only input iterator type.
template <typename T>
struct input_iterator_wrapper_nocopy : input_iterator_wrapper<T> {
    using input_iterator_wrapper<T>::input_iterator_wrapper;

    input_iterator_wrapper_nocopy()
        : input_iterator_wrapper<T>(nullptr, nullptr) {}

    input_iterator_wrapper_nocopy(
        input_iterator_wrapper_nocopy const&) = delete;
    input_iterator_wrapper_nocopy& operator=(
        input_iterator_wrapper_nocopy const&) = delete;

    input_iterator_wrapper_nocopy(input_iterator_wrapper_nocopy&&) = default;
    input_iterator_wrapper_nocopy& operator=(
        input_iterator_wrapper_nocopy&&) = default;

    using input_iterator_wrapper<T>::operator++;

    input_iterator_wrapper_nocopy& operator++() {
        input_iterator_wrapper<T>::operator++();
        return *this;
    }
};

// An input iterator type with an rvalue reference type.
template <typename T>
struct input_iterator_wrapper_rval : input_iterator_wrapper<T> {
    using input_iterator_wrapper<T>::input_iterator_wrapper;

    using input_iterator_wrapper<T>::operator++;

    input_iterator_wrapper_rval& operator++() {
        input_iterator_wrapper<T>::operator++();
        return *this;
    }

    T&& operator*() const {
        return std::move(input_iterator_wrapper<T>::operator*());
    }
};

// A type meeting the minimum std::range requirements
template <typename T, template <typename> class Iter>
class test_range {
    // Exposes the protected default constructor of Iter<T> if needed.  This
    // is needed only when Iter is input_iterator_wrapper or
    // output_iterator_wrapper, because legacy forward iterators and beyond
    // are already default constructible.
    struct iterator : Iter<T> {
        using Iter<T>::Iter;

        using Iter<T>::operator++;

        iterator& operator++() {
            Iter<T>::operator++();
            return *this;
        }
    };

    template <typename I>
    struct sentinel {
        T* end;

        friend bool operator==(sentinel const& s, I const& i) noexcept {
            return s.end == i.ptr;
        }

        friend auto operator-(sentinel const& s, I const& i) noexcept
        requires std::random_access_iterator<I>
        {
            return std::iter_difference_t<I>(s.end - i.ptr);
        }

        friend auto operator-(I const& i, sentinel const& s) noexcept
        requires std::random_access_iterator<I>
        {
            return std::iter_difference_t<I>(i.ptr - s.end);
        }
    };

protected:
    auto get_iterator(T* p) {
        if constexpr (std::default_initializable<Iter<T>>)
            return Iter<T>(p, &bounds);
        else
            return iterator(p, &bounds);
    }

public:
    test_range(T* first, T* last) : bounds(first, last) {}

    template <size_t N>
    explicit test_range(T (&arr)[N]) : test_range(arr, arr + N) {}

    auto begin() & { return get_iterator(bounds.first); }

    auto end() & {
        using I = decltype(get_iterator(bounds.last));
        return sentinel<I>{bounds.last};
    }

    typename Iter<T>::ContainerType bounds;
};

// A move-only type meeting the minimum std::range requirements
template <typename T, template <typename> class Iter>
struct test_range_nocopy : test_range<T, Iter> {
    test_range_nocopy(T* first, T* last) : test_range<T, Iter>(first, last) {}

    test_range_nocopy(test_range_nocopy&&) = default;
    test_range_nocopy& operator=(test_range_nocopy&&) = default;
};

template <typename T>
using test_contiguous_range = test_range<T, contiguous_iterator_wrapper>;
template <typename T>
using test_random_access_range = test_range<T, random_access_iterator_wrapper>;
template <typename T>
using test_bidirectional_range = test_range<T, bidirectional_iterator_wrapper>;
template <typename T>
using test_forward_range = test_range<T, forward_iterator_wrapper>;
template <typename T>
using test_input_range = test_range<T, input_iterator_wrapper>;
template <typename T>
using test_input_range_nocopy =
    test_range_nocopy<T, input_iterator_wrapper_nocopy>;
template <typename T>
using test_output_range = test_range<T, output_iterator_wrapper>;

// A type meeting the minimum std::sized_range requirements
template <typename T, template <typename> class Iter>
struct test_sized_range : test_range<T, Iter> {
    using test_range<T, Iter>::test_range;

    size_t size() const noexcept { return this->bounds.size(); }
};

template <typename T>
using test_contiguous_sized_range =
    test_sized_range<T, contiguous_iterator_wrapper>;
template <typename T>
using test_random_access_sized_range =
    test_sized_range<T, random_access_iterator_wrapper>;
template <typename T>
using test_bidirectional_sized_range =
    test_sized_range<T, bidirectional_iterator_wrapper>;
template <typename T>
using test_forward_sized_range = test_sized_range<T, forward_iterator_wrapper>;
template <typename T>
using test_input_sized_range = test_sized_range<T, input_iterator_wrapper>;
template <typename T>
using test_output_sized_range = test_sized_range<T, output_iterator_wrapper>;

// A type meeting the minimum std::sized_range requirements, and whose end()
// returns a sized sentinel.
template <typename T, template <typename> class Iter>
struct test_sized_range_sized_sent : test_sized_range<T, Iter> {
    using test_sized_range<T, Iter>::test_sized_range;

    template <typename I>
    struct sentinel {
        T* end;

        friend bool operator==(sentinel const& s, I const& i) noexcept {
            return s.end == i.ptr;
        }

        friend std::iter_difference_t<I> operator-(
            sentinel const& s, I const& i) noexcept {
            return std::iter_difference_t<I>(s.end - i.ptr);
        }

        friend std::iter_difference_t<I> operator-(
            I const& i, sentinel const& s) noexcept {
            return std::iter_difference_t<I>(i.ptr - s.end);
        }
    };

    auto end() & {
        using I = decltype(this->get_iterator(this->bounds.last));
        return sentinel<I>{this->bounds.last};
    }
};

// test_range and test_sized_range do not own their elements, so they model
// std::ranges::borrowed_range.  This file does not define specializations of
// std::ranges::enable_borrowed_range, so that individual tests can decide
// whether or not to do so.
// This is also true for test_container, although only when it has forward
// iterators (because output_iterator_wrapper and input_iterator_wrapper are
// not default constructible so do not model std::input_or_output_iterator).

// Test for basic properties of C++20 16.3.3.6 [customization.point.object].
template <typename T>
constexpr bool is_customization_point_object(T&) noexcept {
    // A [CPO] is a function object with a literal class type.
    static_assert(std::is_class_v<T> || std::is_union_v<T>);
    // static_assert(__is_literal_type(T));

    // The type of a [CPO], ignoring cv-qualifiers, shall model semiregular.
    static_assert(std::semiregular<std::remove_cv_t<T>>);

    return true;
}
} // namespace tests
RXX_DEFAULT_NAMESPACE_END
