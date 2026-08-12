vim9script

if exists('b:current_syntax')
    finish
endif

# Comments
syntax match pebbleComment '/.*$'

# Shebang
syntax match pebbleShebang '^#!.*$'

# Keywords
syntax keyword pebbleKeyword Func New End If Escape

# Function name after Func
syntax match pebbleFunction '\h\w*' contained
syntax keyword pebbleFunc Func nextgroup=pebbleFunction skipwhite

# Variable names
syntax match pebbleVariable '\h\w*'

# Variable references
syntax match pebbleReference '{\h\w*}'
syntax match pebbleReference '<\h\w*>'

# VM variables
syntax match pebbleVMVariable '\<VM[A-Z0-9_]*\>'

# Numbers
syntax match pebbleNumber '\<[0-9]\+\>'

# Strings
syntax region pebbleString start='"' skip='\\"' end='"'
syntax region pebbleString start="'" skip="\\'" end="'"

# Blocks
syntax region pebbleBlock start='{' end='}' contains=pebbleKeyword,pebbleFunction,pebbleVariable,pebbleReference,pebbleVMVariable,pebbleNumber,pebbleString,pebbleComment

# Highlighting
highlight default link pebbleComment Comment
highlight default link pebbleShebang PreProc
highlight default link pebbleKeyword Keyword
highlight default link pebbleFunc Keyword
highlight default link pebbleFunction Function
highlight default link pebbleVariable Identifier
highlight default link pebbleReference Special
highlight default link pebbleVMVariable Special
highlight default link pebbleNumber Number
highlight default link pebbleString String

b:current_syntax = 'pebble'
