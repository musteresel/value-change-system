#pragma once

#include "IValueT.h"

#include <utility>

namespace Values {

/// \brief Value storage with a buffer for changes.
///
/// This class provides a value which can be changed, but changes are only
/// finally committed after all observers have been called. Observers have
/// (read) access to both the current ("now") value and the next ("then")
/// value.
///
/// The choice of the buffer type (FIFO, LIFO, Priority queues, ...) will have
/// a drastic influence on what you can do with this class.
///
/// The buffer type must have a the following 3 member functions: first(),
/// returning a reference to the "first" element; drop_first(), which removes
/// that element from the buffer; and insert(V) which inserts the given value
/// into the buffer.
///
template <typename T, typename BufferType,
          template <class...> class ContainerType>
struct BufferedValue
    : public IValueT<BufferedValue<T, BufferType, ContainerType>,
                     ContainerType> {
protected:
  /// \brief The current ("now") stored value.
  ///
  T value_;

  /// \brief The internal buffer of next ("then") values.
  ///
  BufferType buffer_;

  /// \brief Called after all observers are done, completes the change.
  ///
  /// This function is called by the change manager after all observers have
  /// been called. This can be either all observers of this value, or all of a
  /// "set" of value changes, depending on the change manager implementation.
  ///
  virtual void completeChange() override {
    value_ = then();
    buffer_.drop_first();
  }

public:
  template <typename... Args>
  explicit BufferedValue(Args &&...args)
      : value_(std::forward<Args>(args)...) {}

  /// \brief Stores the change to the buffer.
  ///
  template <typename X> void rawChangeTo(X &&x) {
    buffer_.insert(std::forward<X>(x));
  }

  /// \brief Access to the current "now" value.
  ///
  T const &now() const { return value_; }

  /// \brief Access to the next "then" value.
  ///
  T const &then() const { return buffer_.first(); }
};

} // namespace Values
