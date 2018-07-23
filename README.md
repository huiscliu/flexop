# Flexop: A Flexible Command Line Parsing Framework

In many applications, such as "ls (a Linux command to list files)", it accepts lots of command line parameters: 1) **-a**, **--all**: do not ignore entries starting with; 2) **-l**: use a long listing format. Command line options are important tools for manupulating applications and for automation of work flow.


# Credit
The initial code was from PHG project (option.h and option.c, http://lsec.cc.ac.cn/phg/), which was a parallel library for adaptive finite element methods and adaptive finite volume methods. It has been developed for more than 10 years and it has been applied in various areas.

# Purpose
The purpose of this project is to provide a standalone command line parsing framework under Linux, Unix and Mac such that any application can easily support command line options by using this library.

This project has refactorized original code, deleted some types, changed work flow, and added new types have.

The following demo shows options for integer, floating point number, string, vector of integer, vector of floating point number and vector of strings, and details can be found in example/example.c:
```
./example -i 1 -f 1.2 -s "hi" -vi "1 2 3 5" -vf "1.3 2e-4 3.333" -vs "hi jill and jack"

```
