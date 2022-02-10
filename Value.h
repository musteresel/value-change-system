#pragma once

#include "IValueT.h"

#include <utility>

namespace Values {

/// \brief Single value which can change.
///
/// Change it via the changeTo member function. Access the value via implicit
/// conversion or the get member function.
///
template <typename T, template <class...> class ContainerType>
struct Value : public IValueT<Value<T, ContainerType>, ContainerType> {
protected:
  /// \brief The stored value.
  ///
  T value_;

public:
  template <typename... Args>
  explicit Value(Args &&...args) : value_(std::forward<Args>(args)...) {}

  ///\brief Apply a change; for this simple Value class this directly updates
  /// the stored value.
  ///
  template <typename X> void rawChangeTo(X &&x) { value_ = std::forward<X>(x); }

  /// \brief Access the stored value; const because changes need to be commited
  /// via changeTo.
  ///
  T const &get() const { return value_; }

  /// \brief Access to the stored data via implicit conversion.
  ///
  operator T const &() const { return get(); }
};

} // namespace Values
