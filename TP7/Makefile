tp7.exe: tp7.c
	clear
	gcc -g -static -Wall tp7.c -o tp7.exe

run: tp7.exe
	./tp7.exe

run.out: tp7.exe
	./tp7.exe > output.out

socket_nc: socket_nc.exe
	./socket_nc.exe

socket_nc.exe: socket_nc.c
	clear
	gcc -g -static -Wall socket_nc.c -o socket_nc.exe

clean:
	clear
	rm -f tp7.exe output.out

debug: tp7.exe
	gdb ./tp7.exe