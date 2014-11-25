#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2014 <+YOU OR YOUR COMPANY+>.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from optparse import OptionParser, OptionGroup
import os as os
import fileinput
import sys


def setup_parser():
    """ Init the option parser. If derived classes need to add options,
    override this and call the parent function. """
    parser = OptionParser(add_help_option=False)
    parser.usage = '%prog ' + "random name" + ' [options] <PATTERN> \n' + \
                   ' Call "%prog ' + "random name" + '" without any options to run it interactively.'
    ogroup = OptionGroup(parser, "General options")
    ogroup.add_option("-h", "--help", action="help", help="Displays this help message.")
    return parser


def main():
    parser = setup_parser()
    (options, args) = parser.parse_args()
    print "options: ", options
    print "args: ", args
    if len(args) != 2:
        print "wrong number of arguments ", len(args)
    name = _get_module_name()
    if name == None:
        print "no module found"
        return
    old = args[0]
    new = args[1]
    # check once more if the given block name does exist.
    if not _run_swig(name, old, new):
        return

    _run_grc(name, old, new)
    _run_python(name, old, new)
    _run_lib(name, old, new)
    _run_include(name, old, new)


def _get_module_name():
    '''
    auto parse that always!
    '''
    cwd = os.getcwd()
    t = cwd[cwd.rfind('/') + 1:]
    prefix = 'gr-'
    name = t[t.find(prefix) + len(prefix):]
    print name
    swig_file_name = name + '_swig.i'
    if os.path.isfile(os.path.join(cwd, 'swig', swig_file_name)):
        print "found: ", os.path.join(cwd, 'swig', swig_file_name)
    else:
        name = None

    return name


def _run_swig(name, first, second):
    print "\n\n_run_swig: ", name, first, second
    is_file = False
    rfile = './swig/' + name + '_swig.i'
    with open(rfile, 'r') as file:
        for line in file:
            if line.find(first + '.h') > -1:
                is_file = True
    if is_file:
        _run_infile_replace(rfile, first, second)
    return is_file


def _run_grc(name, first, second):
    print "\n\n_run_grc: ", name, first, second
    filename = './grc/' + name + '_' + first + '.xml'
    _run_infile_replace(filename, first, second)
    _run_cmakelists('./grc/', first, second)
    _run_file_rename('./grc/', first, second)


def _run_python(name, first, second):
    print "\n\n_run_python: ", name, first, second
    filename = './python/qa_' + first + '.py'
    print filename
    _run_infile_replace(filename, first, second)

    _run_cmakelists('./python/', first, second)
    _run_file_rename('./python/', first, second)


def _run_lib(name, first, second):
    print "\n\n_run_lib: ", name, first, second
    ccfile = './lib/' + first + '_impl.cc'
    hfile = './lib/' + first + '_impl.h'
    _run_infile_replace(ccfile, first, second)
    _run_infile_replace(hfile, first, second)
    _run_infile_replace(hfile, first.upper(), second.upper())  # take care of include guards
    _run_cmakelists('./lib/', first, second)
    _run_file_rename('./lib/', first, second)


def _run_include(name, first, second):
    print "\n\n_run_include: ", name, first, second
    path = './include/' + name + '/'
    filename = path + first + '.h'
    _run_infile_replace(filename, first, second)
    _run_infile_replace(filename, first.upper(), second.upper())
    _run_cmakelists(path, first, second)
    _run_file_rename(path, first, second)


def _run_cmakelists(path, first, second):
    filename = path + 'CMakeLists.txt'
    print "_run_cmakelists: ", filename
    _run_infile_replace(filename, first, second)


def _run_file_rename(path, first, second):
    files = os.listdir(path)
    for file in files:
        if file.find(first) > -1:
            nl = file.replace(first, second)
            src = path + file
            dst = path + nl
            print "_run_file_rename: ", src, ' --> ', dst
            os.rename(src, dst)


def _run_infile_replace(rfile, first, second):
    print "_run_infile_replace: ", rfile
    if not os.path.isfile(rfile):
        return False
    # with open(rfile, 'r') as file:
    # with fileinput.input(rfile, inplace=1) as file:
    for line in fileinput.input(rfile, inplace=1):
        if line.find(first) > -1:
            old = line
            line = line.replace(first, second)
            sys.stdout.write(old[:-1], ' --> ', line[:-1])


if __name__ == '__main__':
    main()