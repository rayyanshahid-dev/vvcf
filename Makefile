
SRCDIR = src/
SRCS   = src/*.c
CC 	   = gcc
TARGET = vvcf

CFLAGS = -Wall -Wextra -O3 
LDFLAGS = 

# Default target
all: $(TARGET) 
 
# Rule to build the target executable
$(TARGET): $(SRCS)                                                                                                                                                                                     
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)   

run:
	./$(TARGET)

# Clean target to remove the compiled binary
clean:
	rm -f $(TARGET) *.o 

