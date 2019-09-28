make: memsim.c
	gcc -std=c99 memsim.c -o memsim

memsim:
	gcc -std=c99 memsim.c -o memsim

clean:
	rm memsim
