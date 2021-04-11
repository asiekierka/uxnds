local build_dag
build_dag = function(t, dag, i, j, level)
  if dag == nil then
    dag = { }
  end
  if i == nil then
    i = 1
  end
  if j == nil then
    j = #t
  end
  if level == nil then
    level = 0
  end
  if i > j then
    return 
  end
  local mid = math.floor((i + j) / 2)
  dag[t[mid]] = {
    (build_dag(t, dag, i, mid - 1, level + 1)),
    (build_dag(t, dag, mid + 1, j, level + 1))
  }
  return t[mid], dag
end
local append_dag
append_dag = function(node, dag, k)
  local i = k > node and 2 or 1
  local next_node = dag[node][i]
  if next_node then
    return append_dag(next_node, dag, k)
  end
  dag[node][i] = k
  dag[k] = { }
end
local build_dag_from_chars
build_dag_from_chars = function(s, ...)
  local t
  do
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, #s do
      _accum_0[_len_0] = s:sub(i, i)
      _len_0 = _len_0 + 1
    end
    t = _accum_0
  end
  table.sort(t)
  local root, dag = build_dag(t)
  for i = 1, select('#', ...) do
    append_dag(root, dag, (select(i, ...)))
  end
  return root, dag
end
local check_terminals
check_terminals = function(dag, s)
  for i = 1, #s do
    local k = s:sub(i, i)
    assert(not dag[k][1], ('%s has left child node'):format(k))
    assert(not dag[k][2], ('%s has right child node'):format(k))
  end
end
local dump
dump = function(f, root, dag, level)
  if level == nil then
    level = 0
  end
  if dag[root][1] then
    dump(f, dag[root][1], dag, level + 1)
  end
  f:write(('    '):rep(level))
  f:write(root)
  f:write('\n')
  if dag[root][2] then
    return dump(f, dag[root][2], dag, level + 1)
  end
end
local convert = setmetatable({
  ['.'] = 'dot',
  ['\0'] = 'nul'
}, {
  __index = function(self, k)
    return k
  end
})
local write_opcode_tree
do
  local byte_to_opcode = { }
  local byte = false
  for l in assert(io.lines('src/assembler.c')) do
    if l:match('^%s*char%s+ops%[%]%[4%]') then
      byte = 0
    elseif l:match('%}') then
      byte = false
    elseif byte then
      for opcode in l:gmatch('"([A-Z-][A-Z-][A-Z-])"') do
        byte_to_opcode[byte] = opcode
        byte = byte + 1
      end
    end
  end
  local order_to_opcode
  do
    local _accum_0 = { }
    local _len_0 = 1
    for i = 0, #byte_to_opcode do
      if byte_to_opcode[i] ~= '---' then
        _accum_0[_len_0] = byte_to_opcode[i]
        _len_0 = _len_0 + 1
      end
    end
    order_to_opcode = _accum_0
  end
  table.sort(order_to_opcode)
  local root, opcode_to_links = build_dag(order_to_opcode)
  write_opcode_tree = function(f)
    f:write(('\t$tree   .$op-%s ( opcode tree )\n'):format(root:lower()))
    f:write('\t$start\n')
    for i = 0, #byte_to_opcode do
      local opcode = byte_to_opcode[i]
      f:write('\t')
      if opcode ~= '---' then
        f:write(('$op-%s '):format(opcode:lower()))
      else
        f:write('        ')
      end
      for j = 1, 2 do
        if opcode ~= '---' and opcode_to_links[opcode][j] then
          f:write(('.$op-%s '):format(opcode_to_links[opcode][j]:lower()))
        else
          f:write('[ 0000 ] ')
        end
      end
      if i == 0 then
        f:write('$disasm ')
      else
        f:write('        ')
      end
      if opcode ~= '---' then
        f:write(('[ %s ]'):format(opcode))
      else
        f:write('[ ??? ]')
      end
      if i == 0 then
        f:write(' $asm')
      end
      f:write('\n')
    end
  end
end
local type_byte
type_byte = function(size, has_subtree)
  local n1 = has_subtree and '8' or '0'
  local n2
  local _exp_0 = size
  if '1' == _exp_0 then
    n2 = '1'
  elseif '2' == _exp_0 then
    n2 = '3'
  else
    n2 = '0'
  end
  return n1 .. n2
end
local globals = { }
local add_globals
add_globals = function(root, dag, key_to_label, key_to_contents, pad_before, pad_after)
  if pad_before == nil then
    pad_before = ''
  end
  if pad_after == nil then
    pad_after = ''
  end
  for k in pairs(dag) do
    local l = ''
    if k == root then
      l = l .. ('@%s\n'):format(key_to_label('root'):gsub('%s', ''))
    end
    l = l .. ('@%s '):format(key_to_label(k))
    for j = 1, 2 do
      if dag[k][j] then
        l = l .. ('.%s '):format(key_to_label(dag[k][j]))
      else
        l = l .. ('%s[ 0000 ]%s '):format(pad_before, pad_after)
      end
    end
    l = l .. key_to_contents(k)
    l = l .. '\n'
    globals[key_to_label(k):gsub('%s', '')] = l
  end
  globals[key_to_label('root'):gsub('%s', '')] = ''
end
do
  local root, dag = build_dag_from_chars('{}[]%@$;|=~,.^#"\0', '(', ')')
  check_terminals(dag, ')')
  local label_name
  label_name = function(s)
    return ('normal-%-3s'):format(convert[s])
  end
  local label_value
  label_value = function(k)
    return ('[ %02x ]'):format(k:byte())
  end
  add_globals(root, dag, label_name, label_value, '', '   ')
end
do
  local root, dag = build_dag_from_chars('{}', '\0', '(')
  dump(io.stdout, root, dag)
  local label_name
  label_name = function(s)
    if s == '(' then
      return 'normal-(  '
    end
    return ('variable-%s'):format(convert[s])
  end
  local label_value
  label_value = function(k)
    return ('[ %02x ]'):format(k:byte())
  end
  dag['('] = nil
  add_globals(root, dag, label_name, label_value, '', '   ')
end
do
  local root, dag = build_dag_from_chars('{}\0', '(')
  dump(io.stdout, root, dag)
  local label_name
  label_name = function(s)
    if s == '(' then
      return 'normal-(  '
    end
    return ('macro-%-3s'):format(convert[s])
  end
  local label_value
  label_value = function(k)
    return ('[ %02x ]'):format(k:byte())
  end
  dag['('] = nil
  add_globals(root, dag, label_name, label_value, '', '   ')
end
local devices = { }
local add_device
add_device = function(name, fields)
  local field_sizes
  do
    local _tbl_0 = { }
    for k, size in fields:gmatch('(%S+) (%d+)') do
      _tbl_0[k] = size
    end
    field_sizes = _tbl_0
  end
  field_sizes.pad = nil
  local field_names
  do
    local _accum_0 = { }
    local _len_0 = 1
    for k in pairs(field_sizes) do
      _accum_0[_len_0] = k
      _len_0 = _len_0 + 1
    end
    field_names = _accum_0
  end
  table.sort(field_names)
  local root, dag = build_dag(field_names)
  local label_name
  label_name = function(k)
    return ('l-%-14s'):format(name .. '-' .. k)
  end
  local label_value
  label_value = function(k)
    return ('%-17s [ %s ] .%s.%s'):format(('[ %s 00 ]'):format(k), type_byte(field_sizes[k], false), name, k)
  end
  add_globals(root, dag, label_name, label_value, ' ', '        ')
  return table.insert(devices, name)
end
local add_devices
add_devices = function()
  table.sort(devices)
  local root, dag = build_dag(devices)
  local label_name
  label_name = function(k)
    return ('l-%-14s'):format(k)
  end
  local label_value
  label_value = function(k)
    return ('%-17s [ %s ] .%s .l-%s-root'):format(('[ %s 00 ]'):format(k), type_byte(0, true), k, k)
  end
  return add_globals(root, dag, label_name, label_value, ' ', '        ')
end
local filename = 'projects/software/assembler.usm'
local f = assert(io.open(('%s.tmp'):format(filename), 'w'))
local state = 'normal'
local machine = {
  normal = function(l)
    if l:match('%( opcode tree %)') then
      write_opcode_tree(f)
      state = 'opcode'
    elseif l:match('^%@') then
      if l == '@RESET' then
        add_devices()
      end
      for k in l:gmatch('%@(%S+)') do
        if globals[k] then
          f:write(globals[k])
          globals[k] = nil
          return 
        end
      end
      f:write(l)
      return f:write('\n')
    else
      if l:match('^%|%x%x%x%x %;') then
        add_device(l:match('%;(%S+) %{ (.*) %}'))
      end
      f:write(l)
      return f:write('\n')
    end
  end,
  opcode = function(l)
    if not l:match('.') then
      f:write(l)
      f:write('\n')
      state = 'normal'
    end
  end
}
for l in assert(io.lines(filename)) do
  machine[state](l)
end
for _, l in pairs(globals) do
  f:write(l)
end
f:close()
assert(0 == os.execute(('mv %s %s.bak'):format(filename, filename)))
return assert(0 == os.execute(('mv %s.tmp %s'):format(filename, filename)))
