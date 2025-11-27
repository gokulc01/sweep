build:
	g++ sweep.cpp -o sweep.o

install:
	g++ sweep.cpp -o sweep
	mv sweep ~/.local/bin/

clean:
	rm *.o

