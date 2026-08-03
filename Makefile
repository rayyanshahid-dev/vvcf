
SRCDIR = src/
SRCS   = src/*.c
CC 	   = gcc
WCC	   = x86_64-w64-mingw32-gcc
TARGET = vvcf

CFLAGS  = -Wall -Wextra -O3 -march=native -static -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lhts -lpsl -lz -lm -lbz2 -llzma 
LDLIBS  = 

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

report:
	pandoc output/*.md -t pdf -o output/report.pdf 
