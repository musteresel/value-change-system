#pragma once

#include <cassert>
#include <utility>

template<typename EventManager>
struct IValue
{
  friend EventManager;

  protected:
    virtual void handleChange() = 0;

    virtual void completeChange() {};

    void enqueueChange()
    {
      EventManager::getInstance().enqueueIValueChange(*this);
    }
    
    // C.35 from C++ Core Guidelines: Protected non-virtual destructor to
    // avoid destruction of an child class object through a pointer to this
    // (parent) class. 
    ~IValue() {}
};


template<typename T, typename Self, template<class...> class ContainerType, typename EventManager>
struct IValueT : public IValue<EventManager>
{
  public:
    struct Observer
    {
      public:
        virtual void onChange(Self &) = 0;

      protected:
        // C.35 from C++ Core Guidelines: Protected non-virtual destructor to
        // avoid destruction of an child class object through a pointer to this
        // (parent) class.
        ~Observer() {};
    };

    template<typename Class_>
    struct MemFnObserver : public Observer
    {
      Class_ * this_;
      using MemFn = void (Class_::*)(Self &);
      MemFn memfn_;

      MemFnObserver(Class_ * this_, MemFn memfn_) : this_(this_), memfn_(memfn_) {}
      MemFnObserver(Class_ * this_, MemFn memfn_, Self & val) : MemFnObserver(this_, memfn_) {
        val.registerObserver(this);
      }

      void onChange(Self & value) final override
      {
        ((*this_).*memfn_)(value);
      }
    };

    void registerObserver(Observer * const o)
    {
      assert(o != nullptr);
      observers_.push_back(o);
    }

  protected:
    void handleChange() final override
    {
      for (Observer * const o : observers_)
      {
        o->onChange(reinterpret_cast<Self &>(*this));
      }
    }

    ContainerType<Observer *> observers_;

    // C.35 from C++ Core Guidelines: Avoid destruction of a child class object
    // through a pointer with this (parent) class type.
    ~IValueT() {};
};


template<typename T, template<class...> class ContainerType, typename EventManager>
struct Value : public IValueT<T, Value<T, ContainerType, EventManager>, ContainerType, EventManager>
{
  protected:
    T value_;

  public:
    template<typename... Args>
    explicit Value(Args && ... args) : value_(std::forward<Args>(args)...) {}

    template<typename X>
    void changeTo(X && x)
    {
      value_ = std::forward<X>(x);
      IValue<EventManager>::enqueueChange();
    }

    T const & get() const
    {
      return value_;
    }
    operator T const &() const
    {
      return get();
    }
};

template<typename T, typename BufferType, template<class...> class ContainerType, typename EventManager>
struct BufferedValue : public IValueT<T, BufferedValue<T, BufferType, ContainerType, EventManager>, ContainerType, EventManager>
{
  protected:
    T value_;
    BufferType buffer_;

    void completeChange() override
    {
      value_ = then();
      buffer_.pop();
    }

  public:
    template<typename... Args>
    explicit BufferedValue(Args && ... args) : value_(std::forward<Args>(args)...) {}

    template<typename X>
    void changeTo(X && x)
    {
      buffer_.emplace_back(std::forward<X>(x));
      IValue<EventManager>::enqueueChange();
    }

    T const & now() const
    {
      return value_;
    }

    T const & then() const
    {
      return value_;
    }
};

