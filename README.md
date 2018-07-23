# Flexop: A Flexible Command Line Parsing Framework

Initial code was from PHG project (option.h and option.c, http://lsec.cc.ac.cn/phg/). 

In many applications, such as "ls (a Linux command to list files)", it accepts lots of command line parameters: 1) -a, --all: do not ignore entries starting with; 2) -l: use a long listing format.


The purpose of this project is to provide a standalone command line parsing framework to support other applications.

This project has refactorized original code and new types have been supported.

Demo:
```

./example -i 1 -f 1.2 -s "hi"

```
