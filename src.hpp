#pragma once
#include <cstddef>
#include <memory>
#include <utility>
#include <initializer_list>

namespace sjtu {

template <class T>
class vector {
  using alloc_t = std::allocator<T>;
  using traits_t = std::allocator_traits<alloc_t>;

 public:
  using value_type = T;
  using size_type = std::size_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = value_type *;
  using const_iterator = const value_type *;

  vector() noexcept : data_(nullptr), sz_(0), cap_(0) {}

  explicit vector(size_type count, const T &value = T()) : data_(nullptr), sz_(0), cap_(0) {
    if (count) {
      reserve(count);
      for (size_type i = 0; i < count; ++i) emplace_back(value);
    }
  }

  vector(std::initializer_list<T> il) : data_(nullptr), sz_(0), cap_(0) {
    reserve(il.size());
    for (const auto &x : il) emplace_back(x);
  }

  vector(const vector &other) : data_(nullptr), sz_(0), cap_(0) {
    if (other.sz_) {
      reserve(other.sz_);
      for (size_type i = 0; i < other.sz_; ++i) emplace_back(other.data_[i]);
    }
  }

  vector(vector &&other) noexcept : data_(other.data_), sz_(other.sz_), cap_(other.cap_) {
    other.data_ = nullptr;
    other.sz_ = other.cap_ = 0;
  }

  ~vector() {
    clear();
    if (data_) alloc_.deallocate(data_, cap_);
  }

  vector &operator=(const vector &other) {
    if (this == &other) return *this;
    vector tmp(other);
    swap(tmp);
    return *this;
  }

  vector &operator=(vector &&other) noexcept {
    if (this == &other) return *this;
    clear();
    if (data_) alloc_.deallocate(data_, cap_);
    data_ = other.data_;
    sz_ = other.sz_;
    cap_ = other.cap_;
    other.data_ = nullptr;
    other.sz_ = other.cap_ = 0;
    return *this;
  }

  // Element access
  reference operator[](size_type pos) noexcept { return data_[pos]; }
  const_reference operator[](size_type pos) const noexcept { return data_[pos]; }
  pointer data() noexcept { return data_; }
  const_pointer data() const noexcept { return data_; }

  // Iterators
  iterator begin() noexcept { return data_; }
  const_iterator begin() const noexcept { return data_; }
  const_iterator cbegin() const noexcept { return data_; }
  iterator end() noexcept { return data_ + sz_; }
  const_iterator end() const noexcept { return data_ + sz_; }
  const_iterator cend() const noexcept { return data_ + sz_; }

  // Capacity
  bool empty() const noexcept { return sz_ == 0; }
  size_type size() const noexcept { return sz_; }
  size_type capacity() const noexcept { return cap_; }

  void reserve(size_type new_cap) {
    if (new_cap <= cap_) return;
    reallocate(new_cap);
  }

  void shrink_to_fit() {
    if (sz_ < cap_) reallocate(sz_);
  }

  // Modifiers
  void clear() noexcept {
    for (size_type i = sz_; i > 0; --i) traits_t::destroy(alloc_, data_ + (i - 1));
    sz_ = 0;
  }

  void pop_back() {
    if (sz_) {
      --sz_;
      alloc_.destroy(data_ + sz_);
    }
  }

  void push_back(const T &value) {
    if (sz_ == cap_) grow();
    traits_t::construct(alloc_, data_ + sz_, value);
    ++sz_;
  }

  void push_back(T &&value) {
    if (sz_ == cap_) grow();
    traits_t::construct(alloc_, data_ + sz_, std::move(value));
    ++sz_;
  }

  template <class... Args>
  T &emplace_back(Args &&...args) {
    if (sz_ == cap_) grow();
    traits_t::construct(alloc_, data_ + sz_, std::forward<Args>(args)...);
    ++sz_;
    return back();
  }

  reference back() noexcept { return data_[sz_ - 1]; }
  const_reference back() const noexcept { return data_[sz_ - 1]; }

  void swap(vector &other) noexcept {
    std::swap(data_, other.data_);
    std::swap(sz_, other.sz_);
    std::swap(cap_, other.cap_);
  }

 private:
  void grow() {
    size_type new_cap = cap_ ? (cap_ + (cap_ >> 1) + 1) : size_type(1);
    reallocate(new_cap);
  }

  void reallocate(size_type new_cap) {
    pointer new_data = traits_t::allocate(alloc_, new_cap);
    size_type i = 0;
    try {
      for (; i < sz_; ++i) traits_t::construct(alloc_, new_data + i, std::move_if_noexcept(data_[i]));
    } catch (...) {
      for (size_type j = 0; j < i; ++j) traits_t::destroy(alloc_, new_data + j);
      traits_t::deallocate(alloc_, new_data, new_cap);
      throw;
    }
    // destroy old and replace
    for (size_type j = sz_; j > 0; --j) traits_t::destroy(alloc_, data_ + (j - 1));
    if (data_) traits_t::deallocate(alloc_, data_, cap_);
    data_ = new_data;
    cap_ = new_cap;
  }

  pointer data_;
  size_type sz_;
  size_type cap_;
  alloc_t alloc_;
};

}  // namespace sjtu
