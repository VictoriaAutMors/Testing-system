all: 
	mkdir bin -p
	mkdir tmp -p
	gcc ./src/test.c -o ./bin/test -Wall -Wextra -O2
	gcc ./src/judge.c -o ./bin/judge -Wall -Wextra -O2

clean:
	rm -rf bin
	rm -rf tmp
	
