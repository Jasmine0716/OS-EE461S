yash: main.o job.o token.o
	gcc -o yash main.o job.o token.o -lreadline

main.o: main.c
	gcc -c main.c

job.o: job.c
	gcc -c job.c

token.o: token.c
	gcc -c token.c

clean:
	rm -f yash *.o