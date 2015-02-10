#!/bin/sh

# Adapted from
#  http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/

# Appends rules to a compiler-generated dependency file
#  that will prevent "No rule to make target" errors

INFILE=$1
TMPFILE=${INFILE}.tmp

# (we want the existing rule as well)
cp $INFILE $TMPFILE

# The regexes appear to, in order:
#   1. ???
#   2. get rid of everything before the dependency list
#   3. eliminate \ at end of line if present
#   4. skip empty lines?
#   5. stick colon at end
sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$//' \
    -e '/^$/ d' -e 's/$/ :/' < $INFILE >> $TMPFILE

mv $TMPFILE $INFILE
