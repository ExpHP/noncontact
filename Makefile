# still an amateur at this -_-

#=====================================
# Set compiler flags

CXXFLAGS = -Wall -std=c++0x -O3 -g -L/usr/lib -L./bin

#=====================================
# Declare library files

LIBNAME = na3dipth
LIBFILE = bin/lib$(LIBNAME).so

#=====================================
# Declare object files

LIBOBJS = \
	bin/dummy.o \

TESTOBJS = \
	bin/tests.o \

OBJECTS = \
	$(LIBOBJS) \
	$(TESTOBJS) \

#=====================================
# Files removed by `make clean`

EXES = bin/tests.run

REBUILDABLES = $(OBJECTS) $(EXES) $(LIBFILE)

#=====================================
# Standard targets

# make : Make the library
all : $(LIBFILE)

# make check : Compile and run tests
check : bin/tests.run
	$^

# make clean : Remove any compiled files
clean :
	rm -f $(REBUILDABLES)

#=====================================
# Compilation and linking rules

# Linking rule
$(LIBFILE) : $(LIBOBJS)
	g++ $(CXXFLAGS) -fPIC -shared -o $@ $^

# Compilation rule (object)
bin/%.o : src/%.cpp
	g++ $(CXXFLAGS) -fPIC -o $@ -c $<

# Compilation rule (test)
bin/tests.run : $(TESTOBJS) | $(LIBFILE)
	g++ $(CXXFLAGS) -o $@ $^

#=====================================
# Header dependencies (to force rebuilding when headers change...
#                      particularly headers with implementation)

# TODO: Find out why this doesn't always work. >_>

bin/tests.o : src/external-deps/catch.hpp
bin/tests.o : src/potential-lj.hpp
bin/tests.o : src/integrate.hpp
