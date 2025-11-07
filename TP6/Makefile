tp6.exe: tp6.c
	clear
	gcc -g -static -Wall tp6.c -o tp6.exe

part5.exe: part5.c
	clear
	gcc -g -static -Wall part5.c -o part5.exe

run: tp6.exe
	./tp6.exe > output.out
clean:
	clear
	rm -f tp6.exe output.out

debug: tp6.exe
	gdb ./tp6.exe