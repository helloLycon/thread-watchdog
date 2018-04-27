# Thread-Watchdog

## usage

- First of all, call `watchdog_initialize()` to initialize the Watchdog-thread anywhere(Normally, in "main" function)
- call `new_watchdog()` to create a new Watchdog for your thread(must called in the thread to watch)
- call `watchdog_feed()` to feed the dog attached to your thread(the same, must called in thread to watch)

> view test.c for more details

