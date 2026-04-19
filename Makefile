# Environment variable containing flags to pass to the gnu c++ compiler
# -std=c++2a    Specifies that we want support for a new c++ standard
#               so we can use stuff like the [[likely]] annotation
# -Wall			These
# -Wextra			four
# -Wshadow				flags
# -pedantic     			tell the compiler to be very critical of our 
#				code. Code will be scanned for ambiguities and probable
#               bugs and not just actual syntactical or semantic errors.
# -march=native Tells the compiler that the binary only has to run on
#               the exact machine it was compiled on and not general 
#               machines with similar architecture. (more faster code)
CPPFLAGS = -std=c++2a -Wall -Wextra -Wshadow -pedantic -march=native

# Environment variable containing the names of headers that will be used
# with most executables. Without this, make will not know to recompile
# binaries when headers change.
HEADERS = include/VByte.hpp include/timer.hpp

# A fake rule that tells make to not expect to actually create files 
# called "clean" or "debug".
.PHONY: clean debug

# Tells make how to create the "query" file.
# 
# The contents after the colon tells make that the "query" file depends on 
# "query.cpp" and the headers we defined above.
#
# The contents of the TAB indented line(s) tells make how to create the "query"
# file. We tell make to call the gnu c++ compiler (g++) with the specified parameters.
# We use the general CPPFLAGS variable and provide additional flags to define how
# we want the compiler to optimize and what the output file should be called.
main: main.cpp $(HEADERS)
	g++ $(CPPFLAGS) -DNDEBUG -Ofast -o main main.cpp

# Tells make what to do when "make clean" is called.
# Here we simply remove the "query" binary.
clean:
	rm -f main

# Tells make what to do when "make debug" is called.
# Here we compile the binary with different flags to support debugging.
debug: main.cpp $(HEADERS)
	g++ $(CPPFLAGS) -DDEBUG -g -pg -o main main.cpp

# Generic rule that tells make that nothing needs to be done about c++ header files.
%.hpp:

# There is a lot more to compiling C++. Like third party library inclusions and 
# splitting compilation into multiple object code files to be linked and so on.
# For the purposes of this course this simplified idea presented in this Makefile
# is sufficient.
