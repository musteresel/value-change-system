#pragma once

namespace Values {

/// \brief Interface to a value to be used by the change manager.
///
struct IValue {
  /// \brief Actually handles the changes by calling observers.
  ///
  /// \note Implemented in child class which still has type information in
  /// order to call typed observers.
  ///
  virtual void handleChange() = 0;

protected:
  /// \brief Called by concrete subclasses to actually work with the
  /// ChangeManager.
  ///
  template <typename ChangeManager> void enqueueChangeWith(ChangeManager &cm) {
    cm.enqueueIValueChange(*this);
  }

  // C.35 from C++ Core Guidelines: Protected non-virtual destructor to
  // avoid destruction of an child class object through a pointer to this
  // (parent) class.
  ~IValue() {}
};

} // namespace Values
