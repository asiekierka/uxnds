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

expect_failure() {
    cat > 'in.usm'
    if ../bin/debugger asma.rom > asma.log 2>/dev/null || ! grep -qF "${1}" asma.log; then
        echo "error: asma didn't report error ${1} in faulty code"
		tail asma.log
        exit 1
    fi
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
expect_failure 'Invalid hexadecimal: $defg' <<'EOD'
|1000 $defg
EOD
expect_failure 'Invalid hexadecimal: #defg' <<'EOD'
|1000 #defg
EOD
expect_failure 'Address not in zero page: .hello' <<'EOD'
|1000 @hello
	.hello
EOD
expect_failure 'Address outside range: ,hello' <<'EOD'
|1000 @hello
|2000 ,hello
EOD
expect_failure 'Label not found: hello' <<'EOD'
hello
EOD
expect_failure 'Macro already exists: %abc' <<'EOD'
%abc { def }
%abc { ghi }
EOD
expect_failure 'Memory overwrite: SUB' <<'EOD'
|2000 ADD
|1000 SUB
EOD

echo 'All OK'

