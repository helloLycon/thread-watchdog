# Thread-Watchdog

## usage

- First of all, call `watchdog_init()` to initialize the Watchdog-thread anywhere(Normally, in `main()` function)
- call `new_watchdog()` to create a new Watchdog for your thread(must called in the thread to watch)
- call `watchdog_feed()` to feed the dog attached to your thread periodically(the same, must called in thread to watch)
- you can call `delete_watchdog()` to delete the dog attached to your thread(called in your thread)

> view test.c for more details

