# Small Script v2.3.1 for Fallout Van Buren
This repository contains a slightly **modified source code** and modern build setup for **Small v2.3.1** - a specific version dating back to 2003 that was used by *Black Isle Studios* in their proprietary *Jefferson Engine* for development of the original Fallout 3, then codenamed **Van Buren**.

[![GCC Build](https://github.com/AdamLacko/small-script/actions/workflows/cmake-build-gcc.yml/badge.svg)](https://github.com/AdamLacko/small-script/actions/workflows/cmake-build-gcc.yml)
[![Clang Build](https://github.com/AdamLacko/small-script/actions/workflows/cmake-build-clang.yml/badge.svg)](https://github.com/AdamLacko/small-script/actions/workflows/cmake-build-clang.yml)
## About Small
Small is a simple, typeless, 32-bit extension language with a C-like syntax. Execution speed, stability, simplicity and a small footprint were essential design criterions for both the language and the interpreter/abstract machine that a Small program runs on.

An application or tool cannot do or be everything for all users. This not only justifies the diversity of editors, compilers, operating systems and many other software systems, it also explains the presence of extensive configuration options and macro or scripting languages in applications. My own applications have contained a variety of little languages; most were very simple, some were extensive and most needs could have been solved by a general purpose language with a special purpose library.

The Small language was designed as a flexible language for manipulating objects in a host application. The tool set (compiler, abstract machine) were written so that they were easily extensible and would run on different software/hardware architectures.

## Building
The source code authors used Watcom compiler along with Opus Make. I tried this approach and managed to compile the source with [OpenWatcom](https://github.com/open-watcom/open-watcom-v2). However, due to usage of Windows-specific functions, as well as several inconveniences caused by Watcom and its implementation of C, I decided to port this code for modern compilers. As a result, the code in this repository compiles with latest GCC and Clang compilers on both Windows and Linux. MSVC is not tested, but should work as well.

The repository is structured and set up as a standard CMake project. Simply clone or download the repository and open it in your CMake-compliant environment.

## Usage
```
sc.exe <filename> [options]

Options:
         -A<num>  alignment in bytes of the data segment and the stack
         -a       output assembler code (skip code generation pass)
         -C[+/-]  compact encoding for output file (default=+)
         -c8      [default] a character is 8-bits (ASCII/ISO Latin-1)
         -c16     a character is 16-bits (Unicode)
         -d0      no symbolic information, no run-time checks
         -d1      [default] run-time checks, no symbolic information
         -d2      full debug information and dynamic checking
         -d3      full debug information, dynamic checking, no optimization
         -e<name> set name of error file (quiet compile)
         -H<hwnd> window handle to send a notification message on finish
         -i<name> path for include files
         -l       create list file (preprocess only)
         -o<name> set base name of output file
         -P[+/-]  strings are "packed" by default (default=-)
         -p<name> set name of "prefix" file
```
## License
Refer to the original [License](https://github.com/AdamLacko/small-script/blob/main/LICENSE).
