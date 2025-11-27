build:
	g++ sweep.cpp -o sweep

install:
	g++ sweep.cpp -o sweep
	mv sweep ~/.local/bin/

clean:
	rm *.o

