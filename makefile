# Variables
CC = gcc
CFLAGS = -g
LIBS = -lncurses -lzmq

# Source files
SRCS1 = game-server.c remote_char.h message_handler.h aux_global.h
SRCS2 = astronaut-client.c remote_char.h aux_global.h
SRCS3 = outer-space-display.c remote_char.h aux_global.h

# Object files
OBJS1 = game-server.o message_handler.o aux_functions.o
OBJS2 = astronaut-client.o aux_functions.o
OBJS3 = outer-space-display.o aux_functions.o

# Executables
EXE1 = server
EXE2 = client
EXE3 = display

# Default target
all: $(EXE1) $(EXE2) $(EXE3)

# Rule to build server executable
$(EXE1): $(OBJS1)
	$(CC) $(CFLAGS) -o $(EXE1) $(OBJS1) $(LIBS)

# Rule to build human client executable
$(EXE2): $(OBJS2)
	$(CC) $(CFLAGS) -o $(EXE2) $(OBJS2) $(LIBS)

# Rule to build machine client executable
$(EXE3): $(OBJS3)
	$(CC) $(CFLAGS) -o $(EXE3) $(OBJS3) $(LIBS)

# Rule to compile object files
game-server.o: game-server.c remote_char.h aux_global.h message_handler.h
	$(CC) $(CFLAGS) -c game-server.c

astronaut-client.o: astronaut-client.c remote_char.h aux_global.h
	$(CC) $(CFLAGS) -c astronaut-client.c

outer-space-display.o: outer-space-display.c remote_char.h aux_global.h
	$(CC) $(CFLAGS) -c outer-space-display.c

message_handler.o: message_handler.c message_handler.h aux_global.h
	$(CC) $(CFLAGS) -c message_handler.c

aux_functions.o: aux_functions.c aux_global.h 
	$(CC) $(CFLAGS) -c aux_functions.c

# Clean up build files
clean:
	rm -f *.o $(EXE1) $(EXE2)

# Phony targets
.PHONY: all clean