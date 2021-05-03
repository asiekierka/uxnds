import band, bor, lshift, rshift from require 'bit'

spairs = (t) ->
	keys = [ k for k in pairs t ]
	table.sort keys
	i = 0
	->
		i = i + 1
		keys[i], t[keys[i]]

trees = {
	['asma-labels']: {}
	['asma-opcodes']: {}
}

opcodes_in_order = {}

do -- opcodes
	wanted = false
	for l in assert io.lines 'src/assembler.c'
		if l == 'char ops[][4] = {'
			wanted = true
		elseif wanted
			if l == '};'
				break
			for w in l\gmatch '[^%s",][^%s",][^%s",]'
				if w != '---'
					trees['asma-opcodes'][w] = {
						'"%s 00'\format w
						''
					}
				table.insert opcodes_in_order, w
	assert #opcodes_in_order == 32, 'didn\'t find 32 opcodes in assembler code!'

do -- devices -> labels
	add_device = (addr, name, fields) ->
		addr = tonumber addr, 16
		k = if name\match '^Audio%x+$'
			'asma-ldev-Audio'
		else
			'asma-ldev-%s'\format name
		trees['asma-labels'][name] = {
			'"%s 00'\format name
			'00%02x :%s/_entry'\format addr, k
		}
		trees[k] = {}
		addr = 0
		for fname, flen in fields\gmatch '%&(%S+) +%$(%x+)'
			if fname != 'pad'
				trees[k][fname] = {
					'"%s 00'\format fname,
					'00%02x'\format addr
				}
			addr += tonumber flen, 16
	for l in assert io.lines 'projects/examples/blank.usm'
		f = { l\match '^%|(%x%x) +%@(%S+) +%[ (.*) %]' }
		if f[1]
			add_device unpack f


do -- first characters
	representation = setmetatable {
		'&': '26 00 ( & )'
	},
		__index: (c) => "'%s 00"\format c
	process = (label, t) ->
		trees[label] = {}
		for k, v in pairs t
			trees[label]['%02x'\format k\byte!] = {
				representation[k]
				':%s'\format v
			}
	process 'asma-first-char-normal',
		'%': 'asma-macro-define'
		'|': 'asma-pad-absolute'
		'$': 'asma-pad-relative'
		'@': 'asma-label-define'
		'&': 'asma-sublabel-define'
		'#': 'asma-literal-hex'
		'.': 'asma-literal-zero-addr'
		',': 'asma-literal-rel-addr'
		';': 'asma-literal-abs-addr'
		':': 'asma-abs-addr'
		"'": 'asma-raw-char'
		'"': 'asma-raw-word'
		'{': 'asma-ignore'
		'}': 'asma-ignore'
		'[': 'asma-ignore'
		']': 'asma-ignore'
		'(': 'asma-comment-start'
		')': 'asma-comment-end'
	process 'asma-first-char-macro',
		'(': 'asma-comment-start'
		')': 'asma-comment-end'
		'{': 'asma-ignore'
		'}': 'asma-macro-end'
	process 'asma-first-char-comment',
		')': 'asma-comment-end'

traverse_node = (t, min, max, lefts, rights) ->
	i = math.ceil (min + max) / 2
	if min < i
		lefts[t[i]]  = ':&%s'\format traverse_node t, min, i - 1, lefts, rights
	if i < max
		rights[t[i]] = ':&%s'\format traverse_node t, i + 1, max, lefts, rights
	return t[i]

traverse_tree = (t) ->
	lefts, rights = {}, {}
	keys = [ k for k in pairs t ]
	table.sort keys
	lefts, rights, traverse_node keys, 1, #keys, lefts, rights

ptr = (s) ->
	if s
		return ':&%s'\format s
	return ' $2'

ordered_opcodes = (t) ->
	i = 0
	->
		i = i + 1
		v = opcodes_in_order[i]
		if t[v]
			return v, t[v]
		elseif v
			return false, { '"--- 00', '' }

printout = true

fmt = (...) ->
	('\t%-11s %-10s %-12s %-14s %s '\format(...)\gsub ' +$', '\n')

with assert io.open 'projects/software/asma.usm.tmp', 'w'
	for l in assert io.lines 'projects/software/asma.usm'
		if l\match '--- cut here ---'
			break
		\write l
		\write '\n'
	\write '( --- 8< ------- 8< --- cut here --- 8< ------- 8< --- )\n'
	\write '(          automatically generated code below          )\n'
	\write '(          see etc/asma.moon for instructions          )\n'
	\write '\n('
	\write fmt 'label', 'less than', 'greater than', 'key', 'data )'
	\write '\n'
	for name, tree in spairs trees
		\write '@%s\n'\format name
		lefts, rights, entry = traverse_tree tree
		sort_fn = if name == 'asma-opcodes'
			if rights[opcodes_in_order[1]]
				rights[opcodes_in_order[1]] ..= ' &_disasm'
			else
				rights[opcodes_in_order[1]] = ' $2 &_disasm'
			ordered_opcodes
		else
			spairs
		for k, v in sort_fn tree
			label = if k == entry
				'&_entry'
			elseif k
				'&%s'\format k
			else
				''
			\write fmt label, lefts[k] or ' $2', rights[k] or ' $2', unpack v
		\write '\n'
	\write '@asma-heap\n\n'
	\close!
os.execute 'mv projects/software/asma.usm.tmp projects/software/asma.usm'

