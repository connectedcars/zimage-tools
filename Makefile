CC      = gcc
CFLAGS  = -g -Wall -Wextra -std=c11 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
#CFLAGS  = -g -I/usr/include -L/usr/lib -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition 
# -fsanitize=address -fsanitize=leak
RM      = rm -f


# brew build: https://github.com/jd-boyd/python-lzo/issues/23
default: all

all: main

main: src/main.c
	$(CC) $(CFLAGS) src/main.c -o main

clean veryclean:
	$(RM) main
