#pragma once

#include "DefaultChangeManager.h"
#include "IValue.h"

#include <cassert>
#include <utility>

namespace Values {

/// \brief Value with type information, handles observers.
///
/// This class stores observers using a user supplied container type. To handle
/// a change it calls the stored observers, passing the child type (CRTP) of
/// this to allow the observer to directly work with the target type (and not
/// this abstracted interface).
///
template <typename Self, template <class...> class ContainerType>
struct IValueT : public IValue {
  /// \brief Interface to be implemented by observers.
  ///
  struct IObserver {

    /// \brief Member function called when a change is handled.
    ///
    /// This is the function which should by implemented by an observer. It
    /// will be called when the ChangeManager handles the changes, and will get
    /// passed to value which changed.
    ///
    virtual void onChange(Self &) = 0;

  protected:
    // C.35 from C++ Core Guidelines: Protected non-virtual destructor to
    // avoid destruction of an child class object through a pointer to this
    // (parent) class.
    ~IObserver(){};
  };

  /// \brief Special utility observer for member function pointers.
  ///
  /// This utility class allows easy integration of multiple observers into
  /// a single class by use of composition over inheritance. See example for
  /// details.
  ///
  template <typename Class_> struct MemFnObserver : public IObserver {

    /// \brief Captures the this pointer of the (most likely) containing class.
    ///
    Class_ *this_;

    /// \brief Type of the member function pointer
    ///
    using MemFn = void (Class_::*)(Self &);

    /// \brief Member function pointer which will be called in the onChange
    /// member function.
    ///
    MemFn memfn_;

    /// \brief Construct the observer only.
    ///
    MemFnObserver(Class_ *this_, MemFn memfn_) : this_(this_), memfn_(memfn_) {}

    /// \brief Construct and register the observer to changes of the given
    /// value.
    ///
    MemFnObserver(Class_ *this_, MemFn memfn_, Self &val)
        : MemFnObserver(this_, memfn_) {
      val.registerObserver(this);
    }

    /// \brief On change of the value, call the stored member function.
    ///
    void onChange(Self &value) final override { ((*this_).*memfn_)(value); }
  };

  /// \brief Register an observer for this value.
  ///
  void registerObserver(IObserver *const o) {
    assert(o != nullptr);
    observers_.push_back(o);
  }

  /// \brief Change this value with the help of a change manager.
  ///
  template <typename X,
            typename ChangeManager = typename DefaultChangeManager<X>::type>
  void changeTo(X &&x, ChangeManager cm = ChangeManager{}) {
    reinterpret_cast<Self &>(*this).rawChangeTo(std::forward<X>(x));
    this->enqueueChangeWith(cm);
  }

protected:
  /// \brief Called by the ChangeManager, calls all observers.
  ///
  void handleChange() final override {
    for (IObserver *const o : observers_) {
      o->onChange(reinterpret_cast<Self &>(*this));
    }
  }

  /// \brief Storage of pointers to observers, non-owning.
  ///
  ContainerType<IObserver *> observers_;

  // C.35 from C++ Core Guidelines: Avoid destruction of a child class object
  // through a pointer with this (parent) class type.
  ~IValueT(){};
};

} // namespace Values
