all:
	make map red com
map.o : mapper.c
	gcc -c mapper.c -o map.o
red.o : reducer.c
	gcc -c reducer.c -o red.o
com.o : combiner.c
	gcc -c combiner.c -o com.o
clean:
	rm *o map red com
