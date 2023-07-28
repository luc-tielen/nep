build:
	g++ -std=c++20 -o parser parser.cpp -O0 -g3

test: build
	@./parser

clean:
	rm -f *.o parser

.PHONY: build test clean
