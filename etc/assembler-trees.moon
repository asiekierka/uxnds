build_dag = (t, dag = {}, i = 1, j = #t, level = 0) ->
	if i > j
		return
	mid = math.floor (i + j) / 2
	dag[t[mid]] = {
		(build_dag t, dag, i, mid - 1, level + 1)
		(build_dag t, dag, mid + 1, j, level + 1)
	}
	t[mid], dag
append_dag = (node, dag, k) ->
	i = k > node and 2 or 1
	next_node = dag[node][i]
	if next_node
		return append_dag next_node, dag, k
	dag[node][i] = k
	dag[k] = {}
build_dag_from_chars = (s, ...) ->
	t = [ s\sub i, i for i = 1, #s ]
	table.sort t
	root, dag = build_dag t
	for i = 1, select '#', ...
		append_dag root, dag, (select i, ...)
	return root, dag
check_terminals = (dag, s) ->
	for i = 1, #s
		k = s\sub i, i
		assert not dag[k][1], '%s has left child node'\format k
		assert not dag[k][2], '%s has right child node'\format k
dump = (f, root, dag, level = 0) ->
	if dag[root][1]
		dump f, dag[root][1], dag, level + 1
	f\write '    '\rep level
	f\write root
	f\write '\n'
	if dag[root][2]
		dump f, dag[root][2], dag, level + 1

-- deal with opcodes

write_opcode_tree = do
	byte_to_opcode = {}
	byte = false
	for l in assert io.lines 'src/assembler.c'
		if l\match '^%s*char%s+ops%[%]%[4%]'
			byte = 0
		elseif l\match '%}'
			byte = false
		elseif byte
			for opcode in l\gmatch '"([A-Z-][A-Z-][A-Z-])"'
				byte_to_opcode[byte] = opcode
				byte += 1
	order_to_opcode = [ byte_to_opcode[i] for i = 0, #byte_to_opcode when byte_to_opcode[i] != '---' ]
	table.sort order_to_opcode
	root, opcode_to_links = build_dag order_to_opcode
	(f) ->
		for i = 0, #byte_to_opcode
			opcode = byte_to_opcode[i]
			f\write '\t'
			if opcode == root
				f\write '$root   '
			elseif opcode != '---'
				f\write '$op-%s '\format opcode\lower!
			else
				f\write '        '
			for j = 1, 2
				if opcode != '---' and opcode_to_links[opcode][j]
					f\write '.$op-%s '\format opcode_to_links[opcode][j]\lower!
				else
					f\write '[ 0000 ] '
			if i == 0
				f\write '$disasm '
			else
				f\write '        '
			if opcode != '---'
				f\write '[ %s ]'\format opcode
			else
				f\write '[ ??? ]'
			if i == 0
				f\write ' $asm'
			f\write '\n'

type_byte = (size, has_subtree) ->
	n1 = has_subtree and '8' or '0'
	n2 = switch size
		when '1'
			'1'
		when '2'
			'3'
		else
			'0'
	n1 .. n2

globals = {}

add_globals = (root, dag, key_to_label, key_to_contents, pad_before = '', pad_after = '') ->
	for k in pairs dag
		l = ''
		if k == root
			l ..= '@%s\n'\format key_to_label('root')\gsub '%s', ''
		l ..= '@%s '\format key_to_label k
		for j = 1, 2
			if dag[k][j]
				l ..= '.%s '\format key_to_label dag[k][j]
			else
				l ..= '%s[ 0000 ]%s '\format pad_before, pad_after
		l ..= key_to_contents k
		l ..= '\n'
		globals[key_to_label(k)\gsub '%s', ''] = l
	globals[key_to_label('root')\gsub '%s', ''] = ''

do
	root, dag = build_dag_from_chars '{}[]%@$;|=~,.^#"\0', '(', ')'
	check_terminals dag, ')'
-- 	dump io.stdout, root, dag
	convert = {
		['.']: 'dot'
		['\0']: 'nul'
	}
	label_name = (s) -> 'first-char-%-3s'\format convert[s] or s
	label_value = (k) -> '[ %02x ]'\format k\byte!
	add_globals root, dag, label_name, label_value, '  ', '     '

devices = {}

add_device = (name, fields) ->
	field_sizes = { k, size for k, size in fields\gmatch '(%S+) (%d+)' }
	field_sizes.pad = nil
	field_names = [ k for k in pairs field_sizes ]
	table.sort field_names
	root, dag = build_dag field_names
	label_name = (k) -> 'l-%-14s'\format name .. '-' .. k
	label_value = (k) -> '%-17s [ %s ] .%s.%s'\format '[ %s 00 ]'\format(k), type_byte(field_sizes[k], false), name, k
	add_globals root, dag, label_name, label_value, ' ', '        '
	table.insert devices, name

add_devices = ->
	table.sort devices
	root, dag = build_dag devices
	label_name = (k) -> 'l-%-14s'\format k
	label_value = (k) -> '%-17s [ %s ] .%s .l-%s-root'\format '[ %s 00 ]'\format(k), type_byte(0, true), k, k
	add_globals root, dag, label_name, label_value, ' ', '        '

filename = 'projects/software/assembler.usm'

f = assert io.open '%s.tmp'\format(filename), 'w'
-- f = io.stdout
state = 'normal'
machine =
	normal: (l) ->
		if l\match '%$disasm .*%$asm'
			write_opcode_tree f
			state = 'opcode'
		elseif l\match '^%@'
			if l == '@RESET'
				add_devices!
			for k in l\gmatch '%@(%S+)'
				if globals[k]
					f\write globals[k]
					globals[k] = nil
					return
			f\write l
			f\write '\n'
		else
			if l\match '^%|%x%x%x%x %;'
				add_device l\match '%;(%S+) %{ (.*) %}'
			f\write l
			f\write '\n'
	opcode: (l) ->
		if not l\match '%['
			f\write l
			f\write '\n'
			state = 'normal'
for l in assert io.lines filename
	machine[state] l
for _, l in pairs globals
	f\write l
f\close!
assert 0 == os.execute 'mv %s %s.bak'\format filename, filename
assert 0 == os.execute 'mv %s.tmp %s'\format filename, filename

