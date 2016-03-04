FLAGS = -O2 -ansi -Wall -Wextra -c -pthread

main: utilities.o field.o errors.o main.o worker.o storage.o
	mpicc -O2 -pthread main.o utilities.o field.o errors.o worker.o storage.o -o main

storage.o: storage.c
	mpicc $(FLAGS) storage.c
worker.o: worker.c
	mpicc $(FLAGS) worker.c
field.o: field.c 
	mpicc $(FLAGS) field.c 
errors.o: errors.c
	mpicc $(FLAGS) errors.c 
utilities.o: utilities.c
	mpicc $(FLAGS) utilities.c 
main.o: main.c 
	mpicc $(FLAGS) main.c

clean:
	rm *.o main
