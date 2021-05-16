#!/bin/sh
set -e
cd "$(dirname "${0}")/.."
rm -rf asma-test
mkdir asma-test
cd asma-test

build_asma() {
    sed -ne '/^( devices )/,/^( vectors )/p' ../projects/software/asma.usm
    cat <<EOD
|0100 @reset
	;&source-file ;&dest-file ;asma-assemble-file JSR2
	;asma/error LDA2 #0000 NEQ2 JMP BRK
	#0000 DIV

	&source-file "in.usm 00
	&dest-file "out.rom 00

EOD
	sed -ne '/%asma-IF-ERROR/,$p' ../projects/software/asma.usm
}

echo 'Assembling asma with uxnasm'
build_asma > asma.usm
../bin/uxnasm asma.usm asma.rom > uxnasm.log
find ../projects -type f -name '*.usm' -not -name 'blank.usm' | sort | while read F; do
	echo "Comparing assembly of ${F}"
	BN="$(basename "${F%.usm}")"

	if ! ../bin/uxnasm "${F}" "uxnasm-${BN}.rom" > uxnasm.log; then
		echo "error: uxnasm failed to assemble ${F}"
		tail uxnasm.log
		exit 1
	fi
	xxd "uxnasm-${BN}.rom" > "uxnasm-${BN}.hex"

	cp "${F}" 'in.usm'
	if ! ../bin/debugger asma.rom > asma.log; then
		echo "error: asma failed to assemble ${F}, while uxnasm succeeded"
		tail asma.log
		exit 1
	fi
	xxd 'out.rom' > "asma-${BN}.hex"

	diff -u "uxnasm-${BN}.hex" "asma-${BN}.hex"
done
echo 'All OK'

