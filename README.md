# value-change-system (for C++)

This code allows to define values which can be changed, and whose
changes can be observed (e.g. basically change event handlers) by
other code.

The observers / event handlers are not directly called, but rather
the change event is notified to a (user defined) event manager.
This manager can, for example, run on another thread. It will call
all the registered observers.

Apart from a simple value class (which is directly changed) there's
also a BufferedValue class available, which allows for changes to be
committed only on the event manager thread and for the observers to
both see the "old" and the "new" values.

Additionally, there's a Lazy class thrown in for lazy evaluation,
it works nicely with the rest of the system: The change events "push"
the information about changed values, the lazy evaluation "pulls"
the actual values when (and only if) they are needed.

~~~
Change to
  |
  |
  v
Value A --|
          | runs Observer which 
          | updates (CHANGES) the stored 
          | procedure of
          |
          v
     Value (Lazy B)
 (costly evaluation) --|
                       | runs Observer which 
                       | can (if it needs) evaluate
                       | B and which can (if it needs)
                       | then update
                       |
                       v
                    Value C
~~~

