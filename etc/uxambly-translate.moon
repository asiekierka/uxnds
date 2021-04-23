-- Used for porting Uxambly code for use with the old assembler
-- in commit 82f7103a55c21b13f898b20e5d1e174e501bc825 with the
-- assembler that replaced it straight afterwards.

import P, R, S, C, Ct, Cp, V from require 'lpeg'

local labels, filename

opcode_translate =
    PEK2: 'GET'
    POK2: 'PUT'
    LDR: 'PEK2'
    STR: 'POK2'
    LDR2: 'GET2'
    STR2: 'PUT2'

grammar = P {
    'file'
    file: Ct(V'ows' * (V'atom' * V'ows') ^ 0) * Cp!
    ws: C S' \n\t' ^ 1
    ows: C S' \n\t' ^ 0
    atom: V'opcode' + V'comment' + V'variable' + V'addr' + V'literal' + V'setter' + V'getter' + V'short' + V'labeldef' + V'relative' + V'sublabel' + V'data' + V'macro' + V'macroref' + V'rawshort'
    comment: C P'(' * (1-V'ws'*P')') ^ 0 * V'ws' * P')'
    variable: (P';' / -> '@') * C(V'name') * V'ws' * (P'{' / ->'[') * V'ws' * ((P'' / -> '&') * C(V'name') * V'ws' * (P'' / -> '$') * C(V'name') * V'ws') ^ 0 * (P'}' / -> ']') / (...) ->
        var = select 2, ...
        r, w = if var\sub(1, 1) == var\sub(1, 1)\upper!
            ' DEI', ' DEO'
        else
            ' PEK', ' POK'
        for i = 7, select('#', ...), 6
            k = select i, ...
            rr, ww = if '2' == select i + 3, ...
                r .. '2', w .. '2'
            else
                r, w
            labels['~' .. var .. '.' .. k] = '.' .. var .. '/' .. k .. rr
            labels['=' .. var .. '.' .. k] = '.' .. var .. '/' .. k .. ww
            if i == 7
                labels['~' .. var] = '.' .. var .. rr
                labels['=' .. var] = '.' .. var .. ww
        ...
    name: R('az', 'AZ', '09', '__', '--', '++', '**', '//', '??') ^ 1
    addr: C(P'|') * (C(V'hex') / (i) ->
        if i == '0200'
            return '0100'
        if i\match '^01..$'
            return i\sub 3
        return i
    )
    literal: C P'#' * V'hex'
    hex: R('09', 'af', 'AF') ^ 1
    setter: C(P'=' * V'label') / (s) ->
        if not labels[s]
            error 'label not found: %q in %s'\format s, filename
        return labels[s]
    getter: C(P'~' * V'label') / (s) ->
        if not labels[s]
            error 'label not found: %q in %s'\format s, filename
        return labels[s]
    label: R('az', 'AZ', '09', '__', '--', '..', '$$', ']]', '))', '@@', '""', ',,', '##', '||', '{{', '}}', '%%', ';;', '^^', '~~', '==', '//') ^ 1
    short: (P',' / -> ';') * (C(V'label') / (s) -> (s\gsub '%$', '&'))
    rawshort: (P'.' / -> ':') * (C(V'label') / (s) -> (s\gsub '%$', '&'))
    opcode: (C(R'AZ' * R'AZ' * R'AZ' * P'2' ^ -1) / (s) -> opcode_translate[s] or s) * C P'r' ^ -1 * #V'ws'
    labeldef: C P'@' * V'label'
    relative: (P'^' / -> ',') * (C(V'label') / (s) -> (s\gsub '%$', '&'))
    sublabel: (P'$' / -> '&') * (C(V'label') / (s) -> (s\gsub '%$', '&'))
    data: C P'[' * (1-P']') ^ 0 * P']'
    macro: C(P'%' * V'name' * V'ws' * P'{') * V'ws' * (V'atom' * V'ows') ^ 0 * C P'}'
    macroref: C V'name'
}

translate = (_filename) ->
    filename = _filename
    labels = {}
    f = assert io.open filename
    contents = f\read '*a'
    f\close!
    t, len = grammar\match contents
    if len <= #contents
        print '\027[32m%s\027[0;1m%s\027[0m'\format contents\sub(len - 100, len - 1), contents\sub(len, len + 100)
        error 'no match'
    filename = filename\gsub 'attic', 'auto'
    f = assert io.open filename, 'w'
    f\write (table.concat(t)\gsub(' +\n', '\n')\gsub('  +', ' '))
    f\close!
    f = assert io.popen 'bin/assembler %s bin/boot.rom'\format filename
    for l in f\lines!
        print l
        if l == 'Error: Assembly[Failed]'
            os.exit 1
    f\close!
    os.exit 0

translate 'attic/software/left.usm'
os.exit 0

translate 'attic/software/assembler.usm'
translate 'attic/tests/opcodes.usm'
translate 'attic/tests/basics.usm'

-- for k, v in pairs t
--     print k
