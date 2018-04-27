a.out: test.c thread_watchdog.c thread_watchdog.h
	gcc -pthread $^ -o $@

clean: 
	rm -rf a.out
	
