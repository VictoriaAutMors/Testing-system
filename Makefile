all: ./src/test.c
	gcc ./src/test.c -o ./bin/test -Wall -Werror -Wextra -O2
	gcc ./src/judge.c -o ./bin/judge -Wall -Werror -Wextra -O2
%: %.c
	gcc $@.c -o $@ -Wall -Werror -Wextra -lm -g -pipe -fsanitize=leak,address,undefined,null -Ofast
