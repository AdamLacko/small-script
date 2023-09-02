# Small Script v2.3.1
This repository containes a source code and build setup for Small Script v2.3.1 - version dating back to 2003 when it was used by Black Isle Studios in later cancelled videogame Fallout 3, then codenamed Van Buren.
## About Small
Small is a simple, typeless, 32-bit extension language with a C-like syntax. Execution speed, stability, simplicity and a small footprint were essential design criterions for both the language and the interpreter/abstract machine that a Small program runs on.

An application or tool cannot do or be everything for all users. This not only justifies the diversity of editors, compilers, operating systems and many other software systems, it also explains the presence of extensive configuration options and macro or scripting languages in applications. My own applications have contained a variety of little languages; most were very simple, some were extensive and most needs could have been solved by a general purpose language with a special purpose library.

The Small language was designed as a flexible language for manipulating objects in a host application. The tool set (compiler, abstract machine) were written so that they were easily extensible and would run on different software/hardware architectures.

## Building
The source code authors used Watcom compiler along with Opus Make. I tried this approach and managed to compile the source with [OpenWatcom](https://github.com/open-watcom/open-watcom-v2). However, due to several inconveniencies caused by Watcom and its implementation, I decided to try to port the source for a more modern compiler. Thus, the source in this repository compiles with modern GCC 13.1.0.

Project is structured and set up as a standard CMake project. Simply clone or download the repository and open it in your CMake-compliant environment.
