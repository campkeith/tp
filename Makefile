tp: tp.c
	cc -Wall -Ofast -lpthread $^ -o $@

.PHONY: clean
clean:
	rm -f tp
