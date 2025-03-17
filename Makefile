CC = gcc

# OBJ = scanner.o scanner-driver.o parser.o driver.o
OBJ = scanner.o parser.o driver.o symtab.o ast.o ast-print.o
# EXEC = scanner
EXEC = compile

CFLAGS = -Wall -g

# RULES
# rule to build parser
compile: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

# Rule to build the scanner executable
scanner: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

# OBJECT FILES
# compile ast.c
ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

ast-print.o: ast-print.c ast.h
	$(CC) $(CFLAGS) -c ast-print.c

# Compile symtab.c
symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

# Compile driver.c
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

