#!/usr/bin/env python
# generate the file in directory and should be moved to 
# somewhere like the compile root folder of a project 
# to allow libLLVMTracerPass.so to read at runtime.
# file_path = 'PATH/ir_file_where_functions_should_be_skipped.ll'
# have to extract on the -O0 ir file for all functions

import sys

if len(sys.argv) > 1:
    file_path = sys.argv[1]
else:
    print('Usage: python3 <this> <ir_file_path>')
    sys.exit(1)

with open(file_path, 'r') as file:
    # read
    content = file.read()
    # judge
    lines = content.split('\n')
    substrings = []
    for line in lines:
        if line.startswith('define'):
            # func name begain with "define"
            # start after '@', end before '('
            substring = line[line.index('@')+1:line.index('(')]
            substrings.append(substring)
    # output
    output_file_path = './passhelper.skip.funclist'
    with open(output_file_path, 'w') as output_file:
        output_file.write('\n'.join(substrings))
