#!/bin/sh
rm -vf libyatc.a *.o
export CFLAGS="-g -std=c99 -Wall -Wextra $*"
for file in cext.c vmcommon.c  vmcore.c
do
	printf '\t%s\t%s\n' cc $file
	cc -c $CFLAGS -I. -o `basename $file .c`.o $file || exit 3
done
ar crs libyatc.a *.o || exit 1
for testn in test01-splitstring test02-upperlowerreverse test03-variablecontext
do
	rm -f tests/$testn.bin
	printf '\t%s\t%s\n' ld $testn
	cc $CFLAGS -I. -o tests/$testn.bin tests/$testn.c libyatc.a || exit 2
done
exit 0
