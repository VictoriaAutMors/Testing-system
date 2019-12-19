all: 
	mkdir bin -p
	mkdir tmp -p
	gcc ./src/test.c -o ./bin/test -Wall -Wextra -O2
	gcc ./src/judge.c -o ./bin/judge -Wall -Wextra -O2
run:
	./bin/test
clean:
	rm ./bin/judge
	rm ./bin/test
	rmdir bin
	rm ./tmp/*
	rmdir tmp
	
