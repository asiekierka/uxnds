</$objtype/mkfile

TARG=assembler debugger emulator
USM=`{walk -f projects/ | grep '\.usm$' | grep -v blank.usm | grep -v asma.usm}
ROM=${USM:%.usm=%.rom}
CFLAGS=$CFLAGS -I/sys/include/npe
BIN=/$objtype/bin/uxn
HFILES=\
	/sys/include/npe/stdio.h\
	src/devices/apu.h\
	src/devices/mpu.h\
	src/devices/ppu.h\
	src/uxn.h\

CLEANFILES=${TARG:%=bin/%} `{echo $ROM | sed 's,([^ /]+/)+,bin/,g'}

default:V: all

all:V: ${TARG:%=bin/%} $ROM

</sys/src/cmd/mkmany

/sys/include/npe/stdio.h:
	hget https://git.sr.ht/~ft/npe/archive/master.tar.gz | tar xz &&
	cd npe-master &&
	mk install &&
	rm -r npe-master

%.rom: %.usm bin/assembler
	bin/assembler $stem.usm $target && cp $target bin/

bin/assembler:Q: $O.assembler
	mkdir -p bin && cp $prereq $target

bin/debugger:Q: $O.debugger
	mkdir -p bin && cp $prereq $target

bin/emulator:Q: $O.emulator
	mkdir -p bin && cp $prereq $target

$O.assembler: assembler.$O

$O.debugger: debugger.$O uxn.$O

$O.emulator: emulator.$O apu.$O mpu.$O ppu.$O uxn.$O

(assembler|debugger|emulator|uxn)\.$O:R: src/\1.c
	$CC $CFLAGS -Isrc -o $target src/$stem1.c

(apu|mpu|ppu)\.$O:R: src/devices/\1.c
	$CC $CFLAGS -Isrc -o $target src/devices/$stem1.c
