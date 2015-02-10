# still an amateur at this -_-

#=====================================
# Set compiler flags

CXXFLAGS += -Wall -Wextra -pedantic -std=c++0x -O3 -g -L/usr/lib -L./bin

#=====================================
# Dependencies

# FIXME: not portable?
CXXFLAGS += -I/usr/include/eigen3

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
	$(shell scripts/list-test-cases.sh)

OBJECTS = \
	$(LIBOBJS) \
	$(TESTOBJS) \

#=====================================
# Dependency management

# Command to produce a `.d` dependency file.
MAKEDEPEND = $(CXX) $(CXXFLAGS) -MM -MT $@ -o bin/$*.d $<

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
	$(CXX) $(CXXFLAGS) -fPIC -shared -o $@ $^

# Compilation rule (object)
bin/%.o : src/%.cpp
	@$(MAKEDEPEND)
	@scripts/postprocess-depend.sh "bin/$*.d"
	$(CXX) $(CXXFLAGS) -fPIC -o $@ -c $<

# Compilation rule (test)
bin/tests.run : $(TESTOBJS) | $(LIBFILE)
	$(CXX) $(CXXFLAGS) -o $@ $^

#=====================================
# Include auto-generated dependency files

-include $(OBJECTS:.o=.d)
