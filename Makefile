build:
	clang++ -o parser parser.cpp -O0

clean:
	rm -f *.o parser

.PHONY: build clean
