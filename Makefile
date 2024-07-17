# taken from c_utilities
CC=gcc
# CFLAGS=-O3
CFLAGS=-g -Wall
DBGCFLAGS=-g -Wall -DDBG
DBG2CFLAGS=-g -Wall -DDBG2
TDBGCFLAGS=-g -Wall -DDBG # True debug flags!

LIBS=-lgsl -lgslcblas -lm
EXES=gpxrd

# GPX reading.
# these are rough and ready, and make no attempt to parse XML, which is what these files are
# they are specific to application they were downloaded from, i.e. garmoin connect
# also times reflect the application too. i.e. often Garmin Connect is set to Irish time
# so Mungia times for example are an hour behind what they actually were recorded at.
# often, there is no issue for IRL or Canarias.
# first off, simple .. no haversine.
gpxrd: gpxrd.c
	${CC} ${CFLAGS} -o $@ $^ -lm

.PHONY: clean

clean:
	rm -f ${EXES}
