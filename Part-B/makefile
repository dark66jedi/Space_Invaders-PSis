# Variables
CC = gcc
CFLAGS = -g
LIBS = -lncurses -lzmq

# Source files
SRCS1 = game-server.c message_handler.h aux_global.h
SRCS2 = astronaut-client.c aux_global.h
SRCS3 = outer-space-display.c aux_global.h
SRCS4 = astronaut-display-client.c aux_global.h

# Object files
OBJS1 = game-server.o message_handler.o aux_functions.o
OBJS2 = astronaut-client.o aux_functions.o
OBJS3 = outer-space-display.o aux_functions.o
OBJS4 = astronaut-display-client.o aux_functions.o

# Executables
EXE1 = server
EXE2 = client
EXE3 = display
EXE4 = client-display

# Default target
all: $(EXE1) $(EXE2) $(EXE3) $(EXE4)

# Rule to build server executable
$(EXE1): $(OBJS1)
	$(CC) $(CFLAGS) -o $(EXE1) $(OBJS1) $(LIBS)

# Rule to build human client executable
$(EXE2): $(OBJS2)
	$(CC) $(CFLAGS) -o $(EXE2) $(OBJS2) $(LIBS)

# Rule to build machine client executable
$(EXE3): $(OBJS3)
	$(CC) $(CFLAGS) -o $(EXE3) $(OBJS3) $(LIBS)
	
$(EXE4): $(OBJS4)
	$(CC) $(CFLAGS) -o $(EXE4) $(OBJS4) $(LIBS)

# Rule to compile object files
game-server.o: game-server.c aux_global.h message_handler.h
	$(CC) $(CFLAGS) -c game-server.c

astronaut-client.o: astronaut-client.c aux_global.h
	$(CC) $(CFLAGS) -c astronaut-client.c

astronaut-display-client.o: astronaut-display-client.c aux_global.h
	$(CC) $(CFLAGS) -c astronaut-display-client.c

outer-space-display.o: outer-space-display.c aux_global.h
	$(CC) $(CFLAGS) -c outer-space-display.c

message_handler.o: message_handler.c message_handler.h aux_global.h
	$(CC) $(CFLAGS) -c message_handler.c

aux_functions.o: aux_functions.c aux_global.h 
	$(CC) $(CFLAGS) -c aux_functions.c

# Clean up build files
clean:
	rm -f *.o $(EXE1) $(EXE2) $(EXE3) $(EXE4)

# Phony targets
.PHONY: all valgrind clean