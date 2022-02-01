# Copyright (c) 2021 Harman International Industries, Incorporated.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

CXX=g++
LD=ld
CFLAGS=-std=c++17
LFLAGS=
LIBS=-lpthread -lfmt
# LIBS=-lfmt -L/usr/lib/x86_64-linux-gnu/libpthread.so
TESTLIBS=-lgtest -lgtest_main

all: fmtstertest example-json JSONStyle_if_no_fmt_custom_nested_args

fmtstertest.o: fmtstertest.cpp fmtster.h Makefile
	$(CXX) $(CFLAGS) -c $< -o $@

fmtstertest: fmtstertest.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS) $(TESTLIBS) $(LIBS)
	strip $@

example-json.o: example-json.cpp fmtster.h Makefile
	$(CXX) $(CFLAGS) -c $< -o $@

example-json: example-json.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LIBS)
	strip $@

JSONStyle_if_no_fmt_custom_nested_args.o: JSONStyle_if_no_fmt_custom_nested_args.cpp fmtster.h Makefile
	$(CXX) $(CFLAGS) -c $< -o $@

JSONStyle_if_no_fmt_custom_nested_args: JSONStyle_if_no_fmt_custom_nested_args.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LIBS)
	strip $@
