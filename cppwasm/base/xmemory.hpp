#pragma once

#include <memory>
#include <type_traits>

template <typename T>
class observer_ptr {
public:
    using element_type = T;
    using pointer_type = typename std::add_pointer<T>::type;
    using reference_type = typename std::add_lvalue_reference<T>::type;

private:
    pointer_type m_ptr{nullptr};

public:
    constexpr observer_ptr() = default;
    observer_ptr(observer_ptr const &) = default;
    observer_ptr & operator=(observer_ptr const &) = default;
    observer_ptr(observer_ptr &&) = default;
    observer_ptr & operator=(observer_ptr &&) = default;
    ~observer_ptr() = default;

    // pointer-accepting constructors
    constexpr observer_ptr(std::nullptr_t) noexcept {
    }

    explicit constexpr observer_ptr(pointer_type ptr) noexcept : m_ptr{ptr} {
    }

    observer_ptr(std::shared_ptr<element_type> const & ptr) noexcept : m_ptr{ptr.get()} {
    }

    observer_ptr(std::unique_ptr<element_type> const & ptr) noexcept : m_ptr{ptr.get()} {
    }

    // explicit
    // observer_ptr(xobject_ptr_t<element_type> const & ptr) noexcept
    //     : m_ptr{ ptr.get() } {
    // }

    // copying constructor (in addition to compiler-generated copy constructor)
    template <typename U, typename = typename std::enable_if<std::is_convertible<typename std::add_pointer<U>::type, pointer_type>::value>::type>
    observer_ptr(observer_ptr<U> ptr) noexcept : m_ptr{ptr.get()} {
    }

    // 3.2.3, observer_ptr observers
    pointer_type get() const noexcept {
        return m_ptr;
    }

    reference_type operator*() const {
        return *get();
    }

    pointer_type operator->() const noexcept {
        return get();
    }

    explicit operator bool() const noexcept {
        return get() != nullptr;
    }

    // 3.2.4, observer_ptr conversions
    explicit operator pointer_type() const noexcept {
        return get();
    }

    // 3.2.5, observer_ptr modifiers
    pointer_type release() noexcept {
        pointer_type tmp = get();  // TODO(bluecl): [-Wconstexpr-not-const]
        reset();                   // TODO(bluecl): [-Wc++14-extensions]
        return tmp;                // TODO(bluecl): [-Wc++14-extensions]
    }

    void reset(pointer_type ptr = nullptr) noexcept {
        m_ptr = ptr;
    }

    void swap(observer_ptr & ptr) noexcept {
        std::swap(m_ptr, ptr.m_ptr);
    }
};

template <typename T>
constexpr observer_ptr<T> make_observer(std::nullptr_t) noexcept {
    return observer_ptr<T>{};
}

template <typename T>
constexpr observer_ptr<T> make_observer(T * p) noexcept {
    return observer_ptr<T>{p};
}

template <typename T>
observer_ptr<T> make_observer(std::shared_ptr<T> & p) noexcept {
    return observer_ptr<T>{p.get()};
}

template <typename T>
observer_ptr<T> make_observer(std::unique_ptr<T> & p) noexcept {
    return observer_ptr<T>{p.get()};
}

template <typename T, typename U>
bool operator==(observer_ptr<T> p1, observer_ptr<U> p2) {
    return p1.get() == p2.get();
}

template <typename T, typename U>
bool operator!=(observer_ptr<T> p1, observer_ptr<U> p2) {
    return !(p1 == p2);
}

template <typename T>
bool operator==(observer_ptr<T> ptr, std::nullptr_t) noexcept {
    return !ptr;
}

template <typename T>
bool operator==(std::nullptr_t, observer_ptr<T> ptr) noexcept {
    return !ptr;
}

template <typename T>
bool operator!=(observer_ptr<T> ptr, std::nullptr_t) noexcept {
    return bool(ptr);
}

template <typename T>
bool operator!=(std::nullptr_t, observer_ptr<T> ptr) noexcept {
    return bool(ptr);
}

template <typename T, typename U>
bool operator<(observer_ptr<T> p1, observer_ptr<U> p2) {
    return std::less<typename std::common_type<typename std::add_pointer<T>::type, typename std::add_pointer<U>::type>::type>{}(p1.get(), p2.get());
}

template <typename T, typename U>
bool operator>(observer_ptr<T> p1, observer_ptr<U> p2) {
    return p2 < p1;
}

template <typename T, typename U>
bool operator<=(observer_ptr<T> p1, observer_ptr<U> p2) {
    return !(p2 < p1);
}

template <typename T, typename U>
bool operator>=(observer_ptr<T> p1, observer_ptr<U> p2) {
    return !(p1 < p2);
}

template <typename T>
void swap(observer_ptr<T> & p1, observer_ptr<T> & p2) noexcept {
    p1.swap(p2);
}
