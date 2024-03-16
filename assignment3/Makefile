CC = g++
CFLAGS = -Wall -Wextra

# 添加utils.cpp到编译目标
all: play ringmaster

play: play.cpp potato.hpp utils.hpp utils.cpp
	$(CC) $(CFLAGS) $< utils.cpp -o $@

ringmaster: ringmaster.cpp potato.hpp utils.hpp utils.cpp
	$(CC) $(CFLAGS) $< utils.cpp -o $@

clean:
	rm -f play ringmaster