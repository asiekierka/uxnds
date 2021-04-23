</$objtype/mkfile

TARG=assembler debugger emulator
ROM=`{ls -p projects/examples/*.usm | grep -v blank.usm | sed 's/\.usm//g'}
CFLAGS=$CFLAGS -I/sys/include/npe
BIN=/$objtype/bin/uxn
HFILES=\
	/sys/include/npe/stdio.h\
	src/apu.h\
	src/ppu.h\
	src/uxn.h\

CLEANFILES=${TARG:%=bin/%} ${ROM:%=bin/%.rom}

default:V: all

all:V: ${TARG:%=bin/%} ${ROM:%=bin/%.rom}

</sys/src/cmd/mkmany

/sys/include/npe/stdio.h:
	hget https://git.sr.ht/~ft/npe/archive/master.tar.gz | tar xz &&
	cd npe-master &&
	mk install &&
	rm -r npe-master

bin/%.rom: projects/examples/%.usm bin/assembler
	bin/assembler projects/examples/$stem.usm $target

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
