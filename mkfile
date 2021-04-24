</$objtype/mkfile

TARG=assembler debugger emulator
USM=`{walk -f projects/ | grep '\.usm$' | grep -v blank.usm}
ROM=${USM:%.usm=%.rom}
CFLAGS=$CFLAGS -I/sys/include/npe
BIN=/$objtype/bin/uxn
HFILES=\
	/sys/include/npe/stdio.h\
	src/apu.h\
	src/ppu.h\
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

$O.emulator: emulator.$O apu.$O ppu.$O uxn.$O

%.$O: src/%.c
	$CC $CFLAGS -Isrc -o $target src/$stem.c
