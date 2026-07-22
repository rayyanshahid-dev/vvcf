
SRCDIR = src/
SRCS   = src/*.c
CC 	   = gcc
WCC	   = x86_64-w64-mingw32-gcc
TARGET = vvcf

# julia stuff
-include .julia_flags.mk

CFLAGS  = -Wall -Wextra -O3 $(JL_CFLAGS) 
LDFLAGS = $(JL_LDFLAGS)
LDLIBS  = $(JL_LDLIBS)

# Default target
all: $(TARGET) 
 
# Rule to build the target executable
$(TARGET): $(SRCS)                                                                                                                                                                                     
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS) $(LDLIBS)   

run:
	./$(TARGET)

# Clean target to remove the compiled binary
clean:
	rm -f $(TARGET) *.o *.exe 

windows:
	$(WCC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS) $(LDLIBS)
