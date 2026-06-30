CFLAGS = -std=c99 -Wall -Wextra -Iinclude $(shell pkg-config --cflags glfw3) -g
LDFLAGS = $(shell pkg-config --libs glfw3) -ldl -lm -g

main: src/main.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

testbuild: src/example.c src/glad.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean compile_flags.txt

compile_flags.txt:
	# pkg-config --cflags glfw3 | tr ' ' '\n' > $@
	{ echo '-Iinclude'; pkg-config --cflags glfw3; } | tr ' ' '\n' > $@

clean:
	rm -f main
