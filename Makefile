CC = g++
CFLAGS = -Wall -Wextra -c -m64 -O3 -funroll-loops
LFLAGS = -Wall -Wextra -m64

StripTrease: StripTrease.cpp
	$(CC) $(LFLAGS) StripTrease.cpp -o StripTrease
	
clean:
	rm StripTrease