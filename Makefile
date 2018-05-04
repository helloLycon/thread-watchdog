a.out: test.c thread_watchdog.c thread_watchdog.h
	gcc -pthread -Wall $^ -o $@

clean: 
	rm -rf a.out
	
