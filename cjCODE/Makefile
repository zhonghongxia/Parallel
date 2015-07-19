# Indicates compilator to use
CC      = g++

# Specifies compilator options
CFLAGS  = -o3 -fPIC -I. -I/home/onglchen/proenv/jdk8/include \
		  -I/home/onglchen/proenv/jdk8/include/linux \
		  `pkg-config --cflags opencv`  \
		  -D LINUX
LDFLAGS = 
LDLIBS  = `pkg-config --libs opencv`

# Files extensions .cpp, .o
SUFFIXES = .cpp .o
.SUFFIXES: $(SUFFIXES) .

# Name of the main program
#PROG  = search 

# Object files .o necessary to build the main program
OBJS  = fasthessian.o integral.o surf.o utils.o ipoint.o \
		k_means.o vocabularytree.o LRU.o matchpic.o interface.o \
		testtool.o mynative.o 

#lib
libMatch.so: $(OBJS)
	#ar rc ./lib/libMatch.a $(OBJS)
	g++ -rdynamic -shared -o ./lib/libMatch.so $(OBJS) $(LDLIBS)
		
#all: $(PROG)

# Compilation and link
$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -g -o $(PROG) $(OBJS) $(LDLIBS)

.cpp.o:
	$(CC)   $(CFLAGS) -c $< -g -o $@

clean:
	-rm -f *.o
	-rm -f ./lib/*
