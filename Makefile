CC      = gcc
CFLAGS  = -g -Wall -Wextra -std=c11 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
#CFLAGS  = -g -I/usr/include -L/usr/lib -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition 
# -fsanitize=address -fsanitize=leak
RM      = rm -f


# brew build: https://github.com/jd-boyd/python-lzo/issues/23
default: all

all: zimage-extract

zimage-extract: src/zimage-extract.c
	$(CC) $(CFLAGS) src/zimage-extract.c -o zimage-extract

clean veryclean:
	$(RM) zimage-extract
