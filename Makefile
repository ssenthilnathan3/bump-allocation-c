CC       = cc
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -Iinclude -g
ASAN     = -fsanitize=address

SRCS     = block.c header.c
TEST_SRCS = test.c $(SRCS)

.PHONY: all test asan clean

all: test

test: $(TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

asan: $(TEST_SRCS)
	$(CC) $(CFLAGS) $(ASAN) -o test_asan $^

clean:
	rm -f test test_asan *.o
