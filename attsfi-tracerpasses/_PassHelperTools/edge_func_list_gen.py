#! /usr/bin/env python3
# this tool is wirtten for generating the entry function name list of target app.

import sys
output_file_path = './passhelper.edge.funclist'
description = 'Wrtie the edge function names in it manually, project-specifically, and move it into the compile root folder to allow llvm pass read it.'
try:
    with open(output_file_path, 'x'):
        print(f"{output_file_path} created. {description}")
except FileExistsError:
    print(f"{output_file_path} already exists. {description}")