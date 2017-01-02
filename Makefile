# To deal with the Tabs getting converted to spaces, run vim as "vim -u NONE ./Makefile"
# $@ - left side of rule.
# $^ - right side of rule.

DEPS = threads.h threadpool.h

%.o: %.c $(DEPS)
	gcc -c -o $@ $<


OBJ_TT = threadpool.o test_threadpool.o
test_threadpool: $(OBJ_TT)
	gcc -o $@ $^

clean:
	rm -rf ./test_threadpool
	rm -rf ./*.o
