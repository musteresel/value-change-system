#include <functional>
#include <iostream>
#include <list>
#include <vector>

#include "BufferedValue.h"
#include "Lazy.h"
#include "Value.h"

/// The actual "change manager", which could run on e.g. another thread, wait
/// for some changes to accumulate, and then run them.
struct ChangeManager {
  static ChangeManager &getInstance() {
    static ChangeManager instance;
    return instance;
  }

  // Called from "anywhere".
  void enqueueIValueChange(Values::IValue &value) {
    std::cout << "Enqueue change of " << &value << std::endl;
    changes_.push_back(&value);
  }

  std::list<Values::IValue *> changes_;

  // Would need to be called on change manager thread.
  void handleChanges() {
    while (!changes_.empty()) {
      changes_.front()->handleChange();
      changes_.pop_front();
    }
  }
};

// Default constructed on each call to changeTo (if not other change manager
// is supplied). This is just a helper to allow it to work with a global
// singleton. Without this one could do:
//
// ChangeManager cm;
// some_value.changeTo(new_value, cm);
//
struct RefToChangeManagerSingleton {
  void enqueueIValueChange(Values::IValue &value) {
    ChangeManager::getInstance().enqueueIValueChange(value);
  }
};

// Setting the default type of change manager used in the changeTo member
// functions.
namespace Values {
template <typename X> struct DefaultChangeManager<X> {
  using type = RefToChangeManagerSingleton;
};
} // namespace Values

// ---------------- ONLY FOR BUFFERED VALUES NOT USED IN THIS EXAMPLE ---------
// Boiler plate code for a buffered value using a std::vector as container to
// store changes. This can be reduced when e.g. the naming of the required
// member functions are adjusted to the ones available by "normally" used
// backing containers. Or we can just provide a type trait to search for
// common names via SFINAE.
template <typename T> struct vec_wrap : public std::vector<T> {
  void drop_first() { this->erase(this->begin()); }

  T const &first() const { return *(this->cbegin()); }

  void insert(T t) { this->push_back(std::move(t)); }
};

// Shorthand for a buffered value.
template <typename T>
using bval = Values::BufferedValue<T, vec_wrap<T>, std::vector>;
// ----------------------------------------------------------------------------

// Shorthand for a simple (change only) value.
template <typename T> using val = Values::Value<T, std::vector>;

val<int> a(0);

using TypeB = Lazy<int, std::function<int()>>;
val<TypeB> b([]() {
  assert(0);
  return 0;
}); // Default value should not be accessed in this case.

val<int> c(0);

int main(int argc, char **argv) {

  struct A2B {
    decltype(a)::MemFnObserver<A2B> observer_;
    A2B() : observer_(this, &A2B::updateB, a) {}
    void updateB(val<int> &the_a) {
      std::cout << "Change on A (" << the_a.get() << "), causes update of B"
                << std::endl;
      b.changeTo(TypeB([]() {
        std::cout << "Calculating B, this is expensive" << std::endl;
        return 42;
      }));
    }
  } a2b;

  struct B2C {
    decltype(b)::MemFnObserver<B2C> observer_;
    bool use_b_;
    B2C(bool use_b) : observer_(this, &B2C::updateC, b), use_b_(use_b) {}
    void updateC(val<TypeB> &the_b) {
      std::cout << "Change on B, causes update of C" << std::endl;
      c.changeTo(use_b_ ? the_b.get().get() : 21);
    }
  } b2c(argc > 1);

  struct C2_ {
    decltype(c)::MemFnObserver<C2_> observer_;
    C2_() : observer_(this, &C2_::notifyChangeOnC, c) {}
    void notifyChangeOnC(val<int> &the_c) {
      std::cout << "C changed to " << the_c.get() << std::endl;
      auto b_value = b.get().get();
      std::cout << "B is " << b_value << std::endl;
    }
  } c2_;

  a.changeTo(1);

  ChangeManager::getInstance().handleChanges();
}
