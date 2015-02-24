# still an amateur at this -_-

#=====================================
# Set compiler flags

#CXXFLAGS += -Wall -Wextra -pedantic -std=c++0x -O3 -g -L/usr/lib -L./bin
CXXFLAGS += -Wall -Wextra -pedantic -std=c++0x -g -L/usr/lib -L./bin

# Require switches on enums to have complete coverage
CXXFLAGS += -Werror=switch

# Make initialization of aggregate types safe
CXXFLAGS += -Werror=missing-field-initializers

# Catch missing return from non-void function
CXXFLAGS += -Werror=return-type

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

EXAMPLE_EXES = \
	bin/exes/first/first \
	bin/exes/02-ljfit/potential \
	bin/exes/02-ljfit/linear \
	bin/exes/02-ljfit/nonlinear \

OBJECTS = \
	$(LIBOBJS) \
	$(TESTOBJS) \

#=====================================
# Dependency management

# Command to produce a `.d` dependency file.
MAKEDEPEND = $(CXX) $(CXXFLAGS) -MM -MT $@ -o bin/$*.d $<

#=====================================
# Files removed by `make clean`

EXES = bin/tests.run \
	$(EXAMPLE_EXES) \

REBUILDABLES = $(OBJECTS) $(EXES) $(LIBFILE)

#=====================================
# Standard targets

# make : Make the library
all : $(LIBFILE)

# make check : Compile and run tests
check : bin/tests.run
	$^

# make exes : Make example executables
exes : $(EXAMPLE_EXES)

# make clean : Remove any compiled files
clean :
	rm -f $(REBUILDABLES)

#=====================================
# Compilation and linking rules

# Linking rule
$(LIBFILE) : $(LIBOBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -fPIC -shared -o $@ $^

# Compilation rule (object)
bin/%.o : src/%.cpp
	@mkdir -p $(@D)
	@$(MAKEDEPEND)
	@scripts/postprocess-depend.sh "bin/$*.d"
	$(CXX) $(CXXFLAGS) -fPIC -o $@ -c $<

# Compilation rule (test)
bin/tests.run : $(TESTOBJS) | $(LIBFILE)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

#=====================================
# Rules for building the executables

bin/exes/first/first: bin/exes/first/main.o
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

bin/exes/02-ljfit/potential: src/exes/02-ljfit/main.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -DPLOT_POT -DPP_LINEAR $^

bin/exes/02-ljfit/linear: src/exes/02-ljfit/main.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -DPLOT_FIT -DPP_LINEAR $^

bin/exes/02-ljfit/nonlinear: src/exes/02-ljfit/main.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -DPLOT_FIT $^

#=====================================
# Include auto-generated dependency files

-include $(OBJECTS:.o=.d)
