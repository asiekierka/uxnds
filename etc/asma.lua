local spairs
spairs = function(t)
  local keys
  do
    local _accum_0 = { }
    local _len_0 = 1
    for k in pairs(t) do
      _accum_0[_len_0] = k
      _len_0 = _len_0 + 1
    end
    keys = _accum_0
  end
  table.sort(keys)
  local i = 0
  return function()
    i = i + 1
    return keys[i], t[keys[i]]
  end
end
local trees = {
  ['asma-opcodes'] = { }
}
local opcodes_in_order = { }
do
  local wanted = false
  for l in assert(io.lines('src/assembler.c')) do
    if l == 'char ops[][4] = {' then
      wanted = true
    elseif wanted then
      if l == '};' then
        break
      end
      for w in l:gmatch('[^%s",][^%s",][^%s",]') do
        if w ~= '---' then
          trees['asma-opcodes'][w] = {
            ('"%s 00'):format(w),
            ''
          }
        end
        table.insert(opcodes_in_order, w)
      end
    end
  end
  assert(#opcodes_in_order == 32, 'didn\'t find 32 opcodes in assembler code!')
end
do
  local representation = setmetatable({
    ['&'] = '26 00 ( & )'
  }, {
    __index = function(self, c)
      return ("'%s 00"):format(c)
    end
  })
  local process
  process = function(label, t)
    trees[label] = { }
    for k, v in pairs(t) do
      trees[label][('%02x'):format(k:byte())] = {
        representation[k],
        (':%s'):format(v)
      }
    end
  end
  process('asma-first-char-normal', {
    ['%'] = 'asma-macro-define',
    ['|'] = 'asma-pad-absolute',
    ['$'] = 'asma-pad-relative',
    ['@'] = 'asma-label-define',
    ['&'] = 'asma-sublabel-define',
    ['#'] = 'asma-literal-hex',
    ['.'] = 'asma-literal-zero-addr',
    [','] = 'asma-literal-rel-addr',
    [';'] = 'asma-literal-abs-addr',
    [':'] = 'asma-abs-addr',
    ["'"] = 'asma-raw-char',
    ['"'] = 'asma-raw-word',
    ['{'] = 'asma-ignore',
    ['}'] = 'asma-ignore',
    ['['] = 'asma-ignore',
    [']'] = 'asma-ignore',
    ['('] = 'asma-comment-start',
    [')'] = 'asma-comment-end'
  })
  process('asma-first-char-macro', {
    ['('] = 'asma-comment-start',
    [')'] = 'asma-comment-end',
    ['{'] = 'asma-ignore',
    ['}'] = 'asma-macro-end'
  })
  process('asma-first-char-comment', {
    [')'] = 'asma-comment-end'
  })
end
local traverse_node
traverse_node = function(t, min, max, lefts, rights)
  local i = math.ceil((min + max) / 2)
  if min < i then
    lefts[t[i]] = (':&%s'):format(traverse_node(t, min, i - 1, lefts, rights))
  end
  if i < max then
    rights[t[i]] = (':&%s'):format(traverse_node(t, i + 1, max, lefts, rights))
  end
  return t[i]
end
local traverse_tree
traverse_tree = function(t)
  local lefts, rights = { }, { }
  local keys
  do
    local _accum_0 = { }
    local _len_0 = 1
    for k in pairs(t) do
      _accum_0[_len_0] = k
      _len_0 = _len_0 + 1
    end
    keys = _accum_0
  end
  table.sort(keys)
  return lefts, rights, traverse_node(keys, 1, #keys, lefts, rights)
end
local ptr
ptr = function(s)
  if s then
    return (':&%s'):format(s)
  end
  return ' $2'
end
local ordered_opcodes
ordered_opcodes = function(t)
  local i = 0
  return function()
    i = i + 1
    local v = opcodes_in_order[i]
    if t[v] then
      return v, t[v]
    elseif v then
      return false, {
        '"--- 00',
        ''
      }
    end
  end
end
local printout = true
local fmt
fmt = function(...)
  return (('\t%-11s %-10s %-12s %-14s %s '):format(...):gsub(' +$', '\n'))
end
do
  local _with_0 = assert(io.open('projects/software/asma.usm.tmp', 'w'))
  for l in assert(io.lines('projects/software/asma.usm')) do
    if l:match('--- cut here ---') then
      break
    end
    _with_0:write(l)
    _with_0:write('\n')
  end
  _with_0:write('( --- 8< ------- 8< --- cut here --- 8< ------- 8< --- )\n')
  _with_0:write('(          automatically generated code below          )\n')
  _with_0:write('(          see etc/asma.moon for instructions          )\n')
  _with_0:write('\n(')
  _with_0:write(fmt('label', 'less', 'greater', 'key', 'binary'))
  _with_0:write(fmt('', 'than', 'than', 'string', 'data )'))
  _with_0:write('\n')
  for name, tree in spairs(trees) do
    _with_0:write(('@%s\n'):format(name))
    local lefts, rights, entry = traverse_tree(tree)
    local sort_fn
    if name == 'asma-opcodes' then
      if rights[opcodes_in_order[1]] then
        rights[opcodes_in_order[1]] = rights[opcodes_in_order[1]] .. ' &_disasm'
      else
        rights[opcodes_in_order[1]] = ' $2 &_disasm'
      end
      sort_fn = ordered_opcodes
    else
      sort_fn = spairs
    end
    for k, v in sort_fn(tree) do
      local label
      if k == entry then
        label = '&_entry'
      elseif k then
        label = ('&%s'):format(k)
      else
        label = ''
      end
      _with_0:write(fmt(label, lefts[k] or ' $2', rights[k] or ' $2', unpack(v)))
    end
    _with_0:write('\n')
  end
  _with_0:write([[(
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

]])
  _with_0:close()
end
return os.execute('mv projects/software/asma.usm.tmp projects/software/asma.usm')
