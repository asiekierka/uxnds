</$objtype/mkfile

TARG=bin/debugger bin/uxnasm bin/uxnemu
USM=`{walk -f projects/ | grep '\.usm$' | grep -v blank.usm}
ROM=${USM:%.usm=%.rom}
CFLAGS=$CFLAGS -I/sys/include/npe
HFILES=\
	/sys/include/npe/stdio.h\
	src/devices/apu.h\
	src/devices/mpu.h\
	src/devices/ppu.h\
	src/uxn.h\

CLEANFILES=$TARG $ROM

default:V: all

all:V: bin $TARG $ROM

bin:
	mkdir -p bin

/sys/include/npe/stdio.h:
	hget https://git.sr.ht/~ft/npe/archive/master.tar.gz | tar xz &&
	cd npe-master &&
	mk install &&
	rm -r npe-master

%.rom:Q: %.usm bin/uxnasm
	bin/uxnasm $stem.usm $target >/dev/null

bin/debugger: debugger.$O uxn.$O
	$LD -o $target $prereq

bin/uxnasm: assembler.$O
	$LD -o $target $prereq

bin/uxnemu: emulator.$O apu.$O mpu.$O ppu.$O uxn.$O
	$LD -o $target $prereq

(assembler|debugger|emulator|uxn)\.$O:R: src/\1.c
	$CC $CFLAGS -Isrc -o $target src/$stem1.c

(apu|mpu|ppu)\.$O:R: src/devices/\1.c
	$CC $CFLAGS -Isrc -o $target src/devices/$stem1.c

nuke:V: clean

clean:V:
	rm -f *.[$OS] [$OS].??* $TARG $CLEANFILES

%.clean:V:
	rm -f $stem.[$OS] [$OS].$stem $stem

install:QV: all
	exit 'Sorry, there is no install rule yet'
