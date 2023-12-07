example: clean
	g++ example.cpp -o example -g3 -ggdb
	./example

clean:
	rm -rf example
