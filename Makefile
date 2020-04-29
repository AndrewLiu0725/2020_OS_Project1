CFLAG = -fgnu89-inline

all:
	gcc $(CFLAG) main.c -c
	gcc $(CFLAG) scheduler_FIFO.c -c
	gcc $(CFLAG) scheduler_RR.c -c
	gcc $(CFLAG) scheduler_SJF.c -c
	gcc $(CFLAG) scheduler_PSJF.c -c
	gcc $(CFLAG) process.c -c
	gcc main.o scheduler_FIFO.o scheduler_RR.o scheduler_SJF.o scheduler_PSJF.o process.o -o OS_Project1

clean:
	rm -f *.o OS_Project1

