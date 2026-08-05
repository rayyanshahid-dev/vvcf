
SRCDIR = src/
SRCS   = src/*.c
CC 	   = gcc
WCC	   = x86_64-w64-mingw32-gcc
TARGET = vvcf

CFLAGS   = -Wall -Wextra -O3 -march=native -static -std=c99
WCFLAGS  = -Wno-error=implicit-function-declaration
LDFLAGS  = -lhts -lz -lm 
WLDFLAGS = -lbz2 -llzma -lws2_32 -lsystre -ltre -lregex -lintl -liconv
LDLIBS   =  

# -L/usr/local/lib
# -I/usr/local/include

# Default target
all: $(TARGET) 
 
# Rule to build the target executable
$(TARGET): $(SRCS)                                                                                                                                                                                     
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS) $(LDLIBS)   
	@echo "Built on: $$(date)\n"

run:
	./$(TARGET)

# Clean target to remove the compiled binary
clean:
	rm -f $(TARGET) *.o *.exe 

windows:
	$(WCC) $(WCFLAGS) -o $(TARGET) $(SRCS) $(WLDFLAGS) 

report:
	pandoc output/*.md -t pdf -o output/report.pdf \
		--pdf-engine=xelatex \
		-V geometry:margin=1in \
		-V fontsize=11pt
 
