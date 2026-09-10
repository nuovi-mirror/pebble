vim9script

if exists('b:current_syntax')
    finish
endif

# Keywords
syntax keyword rockKeyword . fn if call import export

# Function names
syntax match rockFunction '\h\w*\%(\.\h\w*\)*' contained

# Function declarations/calls
syntax keyword rockFn fn nextgroup=rockFunction skipwhite
syntax keyword rockCallKeyword call nextgroup=rockFunction skipwhite

# Assignment
syntax match rockOperator '='

# Markers
syntax match rockExpression '`.*$'
syntax match rockLiteral '!.*$'
syntax match rockVariable '@\h\w*'
syntax match rockReturn ':'

# Numbers
syntax match rockNumber '\<[0-9]\+\>'

# Comments
syntax match rockComment '/.*$'

# Blocks
syntax region rockBlock start='{' end='}' contains=rockBlock,rockKeyword,rockFn,rockCallKeyword,rockOperator,rockExpression,rockLiteral,rockVariable,rockReturn,rockNumber,rockComment

# Highlighting
highlight default link rockKeyword Keyword
highlight default link rockFn Keyword
highlight default link rockCallKeyword Keyword
highlight default link rockFunction Function
highlight default link rockOperator Operator
highlight default link rockExpression Special
highlight default link rockLiteral String
highlight default link rockVariable Identifier
highlight default link rockReturn Operator
highlight default link rockNumber Number
highlight default link rockComment Comment

b:current_syntax = 'rock'
