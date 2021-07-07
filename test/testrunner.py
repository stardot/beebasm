# =====================================================================================================
#
#   Copyright (C) Charles Reilly 2021
#
#   This file is part of BeebAsm.
#
#   BeebAsm is free software: you can redistribute it and/or modify it under the terms of the GNU
#   General Public License as published by the Free Software Foundation, either version 3 of the
#    License, or (at your option) any later version.
#
#   BeebAsm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
#   even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along with BeebAsm, as
#   COPYING.txt.  If not, see <http://www.gnu.org/licenses/>.
#
# =====================================================================================================

import os
import sys
import subprocess

class TestFailure(Exception):
    '''A test failed'''
    pass

def replace_extension(file_name, ext):
    return os.path.splitext(file_name)[0] + ext

def compare_files(name1, name2):
    with open(name1, 'rb') as file1, open(name2, 'rb') as file2:
        return file1.read() == file2.read()

# Parse a string containing parameters separated by spaces.  Parameters
# may be quoted with double quotes.
def parse_quoted_string(text):
    params = []
    index = 0
    length = len(text)
    while index < length:
        while index < length and text[index].isspace():
            index += 1
        if index < length and text[index] == '\"':
            index += 1
            start_index = index
            while index < length and text[index] != '\"':
                index += 1
            end_index = index
            if index < length:
                index += 1
        else:
            start_index = index
            while index < length and not text[index].isspace():
                index += 1
            end_index = index
        if start_index != end_index:
            params += [text[start_index:end_index]]
    return params

# Read switches from the first line of a beebasm source file, looking for:
# \ beebasm <switches>
def read_beebasm_switches(file_name):
    with open(file_name) as test_file:
        first_line = test_file.readline()
        test_file.close()
        if first_line == '' or first_line[0] != '\\':
            return []
        # readline can return a trailing newline
        if first_line[-1:] == '\n':
            first_line = first_line[:-1]
        params = parse_quoted_string(first_line[1:])
        if params == [] or params[0] != 'beebasm':
            return []
        return params[1:]

def beebasm_args(beebasm, file_name, ssd_name):
    args = [beebasm] + read_beebasm_switches(file_name)
    if ssd_name != None:
        args += ['-do', ssd_name]
    args += ['-v', '-i', file_name]
    return args

def execute_test(beebasm_arg_list):
    print(beebasm_arg_list)
    sys.stdout.flush()
    # Child stderr written to stdout to avoid output interleaving problems
    return subprocess.Popen(beebasm_arg_list, stdout = sys.stdout, stderr = sys.stdout).wait() == 0

def run_test(beebasm, path, file_names, file_name):
    if file_name.endswith('.inc.6502'):
        return

    full_name = os.path.join(path, file_name)
    print('='*70)
    print('TEST: ' + full_name)
    print('='*70)

    failure_test = file_name.endswith('.fail.6502')
    gold_ssd = replace_extension(file_name, '.gold.ssd')
    ssd_name = None
    if gold_ssd in file_names:
        ssd_name = 'test.ssd'

    result = execute_test(beebasm_args(beebasm, file_name, ssd_name))
    if failure_test and result:
        raise TestFailure('Failure test succeeded: ' + full_name)
    elif not failure_test and not result:
        raise TestFailure('Success test failed: ' + full_name)

    if not failure_test and ssd_name != None and not compare_files(gold_ssd, ssd_name):
        raise TestFailure('ssd does not match gold ssd: ' + gold_ssd)

def scan_directory(beebasm):
    for (path, directory_names, file_names) in os.walk('.', topdown = True):
        # Sort directory names; this allows simpler tests to be prioritised
        directory_names.sort()
        file_names.sort()
        cwd = os.getcwd()
        os.chdir(path)
        for file_name in file_names:
            if os.path.splitext(file_name)[1] == '.6502':
                run_test(beebasm, path, file_names, file_name)
        os.chdir(cwd)

def parse_args():
    global verbose

    if len(sys.argv) == 1:
        return True

    if len(sys.argv) > 2:
        return False

    if sys.argv[1] == '-v':
        verbose = True
    else:
        return False

    return True


verbose = False

if not parse_args():
    print("testrunner.py [-v]")
    sys.exit(2)

if os.name == 'nt':
    beebasm = 'beebasm.exe'
else:
    beebasm = 'beebasm'
beebasm = os.path.join(os.getcwd(), beebasm)

os.chdir('test')

original_stdout = sys.stdout
if not verbose:
    sys.stdout = open('testlog.txt', mode = 'w', encoding = sys.stdout.encoding)

try:
    scan_directory(beebasm)
    print("SUCCESS: beebasm tests succeeded", file = original_stdout)
    sys.exit(0)

except TestFailure as e:
    print("FAILURE: " + e.args[0], file = original_stdout)
    sys.exit(1)

