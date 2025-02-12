CC = gcc

# OBJ = scanner.o scanner-driver.o parser.o driver.o
OBJ = scanner.o parser.o driver.o
# EXEC = scanner
EXEC = compile

CFLAGS = -Wall -g

# rule to build parser
compile: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

# # Rule to build the scanner executable
# scanner: $(OBJ)
# 	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

# Compiler driver.c
driver.o: driver.c
	$(CC) $(CFLAGS) -c driver.c

# Compile parser.c
parser.o: parser.c scanner.h
	$(CC) $(CFLAGS) -c parser.c

# Compile scanner.c
scanner.o: scanner.c scanner.h
	$(CC) $(CFLAGS) -c scanner.c

# Compile scanner_driver.c
scanner-driver.o: scanner-driver.c scanner.h
	$(CC) $(CFLAGS) -c scanner-driver.c

# Clean rule to remove executables and object files
clean:
	rm -f $(EXEC) *.o

