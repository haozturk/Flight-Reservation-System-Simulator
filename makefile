out: main.o Seat.o
	g++ -o out main.o Seat.o -lpthread
main.o: main.cpp Seat.h
	g++ -c main.cpp -lpthread
Seat.o: Seat.cpp Seat.h
	g++ -c Seat.cpp -lpthread

clear:
	rm *.o