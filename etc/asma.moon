--
-- Asma tree helper script
--
-- This script updates the trees at the end of projects/software/asma.usm when
-- Uxn's opcode set changes or new runes (first character of tokens) are
-- created, so that new changes in the C assembler can be incorporated rapidly
-- into asma.
--
-- To run, you need Lua or LuaJIT, and just run etc/asma.lua from the top
-- directory of Uxn's git repository:
--
--     lua etc/asma.lua
--
-- This file is written in MoonScript, which is a language that compiles to
-- Lua, the same way as e.g. CoffeeScript compiles to JavaScript. Since
-- installing MoonScript has more dependencies than Lua, the compiled
-- etc/asma.lua is kept in Uxn's repository and will be kept updated as this
-- file changes.
--

spairs = (t) ->
	keys = [ k for k in pairs t ]
	table.sort keys
	i = 0
	->
		i = i + 1
		keys[i], t[keys[i]]

trees = {
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
	\write fmt 'label', 'less', 'greater', 'key', 'binary'
	\write fmt '', 'than', 'than', 'string', 'data )'
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
	\write [[(
	Heap, a large temporary area for keeping track of labels. More complex
	programs need more of this space. If there's insufficient space then the
	assembly process will fail, but having extra space above what the most
	complex program needs provides no benefit.

	This heap, and the buffers below, are free to be used to hold temporary
	data between assembly runs, and do not need to be initialized with any
	particular contents to use the assembler.
)

@asma-heap

|ff00 &end

(
	Buffer for use with loading source code.
	The minimum size is the length of the longest token plus one, which is
	0x21 to keep the same capability of the C assembler.
	Larger sizes are more efficient, provided there is enough
	heap space to keep track of all the labels.
)

@asma-read-buffer

|ff80 &end

(
	Buffer for use with writing output.
	The minimum size is 1, and larger sizes are more efficient.
)

@asma-write-buffer

|ffff &end

]]
	\close!
os.execute 'mv projects/software/asma.usm.tmp projects/software/asma.usm'

