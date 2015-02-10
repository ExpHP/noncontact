#! /bin/sh

# Search src/tests recursively, locate .cpp files, and convert their file paths
#  to the corresponding object path (src/source.cpp -> bin/source.o)
find src/tests -not -type d | grep \\.cpp$ | sed s/^src/bin/ | sed s/\\.cpp$/.o/
