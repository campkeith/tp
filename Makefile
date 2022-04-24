tp: tp.c
	cc -std=c99 -Wall -Ofast -lpthread $^ -o $@

.PHONY: clean
clean:
	rm -f tp
