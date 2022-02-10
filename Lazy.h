#pragma once


template<typename T, typename Fn>
class Lazy {
  mutable union {
    T value_;
  } storage_;
  mutable bool unrealized_ = true;
  Fn fn_;

  public:
    explicit Lazy(Fn f) : fn_(std::move(f)) {}

  T const & get() const {
    if (unrealized_) {
      new (&storage_.value_) T(fn_());
      unrealized_ = false;
    }
    return storage_.value_;
  }

  operator T const &() const {
    return get();
  }
};

template<typename Y>
using rem = std::remove_cv_t<std::remove_reference_t<Y>>;

template<typename T, typename Fn>
auto make_lazy(Fn && fn)
{
  return Lazy<rem<T>, rem<Fn>>(std::forward<Fn>(fn));
}

