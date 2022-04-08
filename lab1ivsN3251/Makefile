CFLAGS=-Wall -Wextra -Werror -O2
TARGETS=lab1test lab1ivsN3251 libivsN3251.so

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -rf *.o $(TARGETS)

lab1test: lab1test.c plugin_api.h
	gcc $(CFLAGS) -o lab1test lab1test.c -ldl

lab1ivsN3251: lab1ivsN3251.c plugin_api.h
	gcc $(CFLAGS) -o lab1ivsN3251 lab1ivsN3251.c -ldl

libivsN3251.so: libivsN3251.c plugin_api.h
	gcc $(CFLAGS) -shared -fPIC -o libivsN3251.so libivsN3251.c -ldl -lm


