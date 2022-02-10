#include "Value.h"
#include "Lazy.h"
#include <vector>
#include <iostream>
#include <functional>


struct EventManager {
  static EventManager & getInstance() {
    static EventManager instance;
    return instance;
  }

  void enqueueIValueChange(IValue & value) {
    std::cout << "Enqueue change of " << &value << std::endl;
    changes_.emplace_back(std::ref(value));
  }

  std::vector<std::reference_wrapper<IValue>> changes_;

  void handleChanges() {
    for (auto v : changes_) {
      v.get().handleChange();
    }
    for (auto v : changes_) {
      v.get().completeChange();
    }
    changes_.clear();
  }
};

struct EventManagerRef {
  void enqueueIValueChange(IValue & value) {
    EventManager::getInstance().enqueueIValueChange(value);
  }
};

template<typename X>
struct DefaultEventManager<X> {
  using type = EventManagerRef;
};


template<typename T>
using val = Value<T, std::vector>;

val<int> foo(5);
val<double> bar(3.4);

struct Dummy : val<int>::Observer
{
  void onChange(val<int> &) override {
    std::cout << "Changed!" << std::endl;
  }
};

int main()
{
  Dummy d;
  foo.registerObserver(&d);
  val<std::vector<char>> baz;
  foo.changeTo(42);
  baz.changeTo(std::vector<char>{3,2,1});

  auto l = make_lazy<int>(std::function<int()>{[]() { 
    std::cout << "realized!" << std::endl;
    return 42;
  }});

  val<decltype(l)> xyz(l);
  xyz.changeTo(make_lazy<int>(std::function<int()>{[]() { std::cout << "foo" << std::endl; return 21; }}));

  struct : val<decltype(l)>::Observer {
    void onChange(val<decltype(l)> & v) override {
      std::cout << "Changed the lazy thing!" << std::endl;
      std::cout << "is now: " << v.get().get() << std::endl;
    };
  } obs;
  xyz.registerObserver(&obs);

  struct U {
    val<int>::MemFnObserver<U> fooObserver_;
    val<int>::MemFnObserver<U> foo2Observer_;

    U() : fooObserver_(this, &U::fooCalled), foo2Observer_(this, &U::fooCalledAgain, foo)
    {
      foo.registerObserver(&fooObserver_);
    }

    void fooCalled(val<int> & f) {
      std::cout << "fooCalled" << std::endl;
    }
    void fooCalledAgain(val<int> & f) {
      std::cout << "fooCalledAgain" << std::endl;
    }
  };

  U u;

  EventManager::getInstance().handleChanges();
}

