CC = gcc

OBJ = scanner.o scanner-driver.o
EXEC = scanner

CFLAGS = -Wall -g

# Rule to build the scanner executable
scanner: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

# Compile scanner.c
scanner.o: scanner.c scanner.h
	$(CC) $(CFLAGS) -c scanner.c

# Compile scanner_driver.c
scanner-driver.o: scanner-driver.c scanner.h
	$(CC) $(CFLAGS) -c scanner-driver.c

# Clean rule to remove executables and object files
clean:
	rm -f $(EXEC) $(OBJ)

