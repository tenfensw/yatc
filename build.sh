#!/bin/sh
rm -vf libyatc.a *.o yatc
export CFLAGS="-g -std=c99 -Wall -Wextra $*"
iofile=vmio_cli.c
test "$KDE" != "" && iofile=vmio_kde.c
for file in cext.c vmcommon.c vmio.c $iofile vmcore.c tinyexpr/tinyexpr.c vmexpr.c
do
	printf '\t%s\t%s\n' cc $file
	cc -c $CFLAGS -I. -Itinyexpr -o `basename $file .c`.o $file || exit 3
done
ar crs libyatc.a *.o || exit 1
for testn in test01-splitstring test02-upperlowerreverse test03-variablecontext
do
	rm -f tests/$testn.bin
	printf '\t%s\t%s\n' ld $testn
	cc $CFLAGS -I. -o tests/$testn.bin tests/$testn.c libyatc.a -lm || exit 2
done
printf '\t%s\t%s\n' ld yatc
cc $CFLAGS -I. -o yatc main.c libyatc.a -lm || exit 4
exit 0
