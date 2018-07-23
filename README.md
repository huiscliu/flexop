# Flexop: A Flexible Command Line Parsing Framework

In many applications, such as "ls (a Linux command to list files)", it accepts lots of command line parameters: 1) **-a**, **--all**: do not ignore entries starting with; 2) **-l**: use a long listing format. Command line options are important tools for manupulating applications and for automation of work flow.


# Credit
The initial code was from PHG project (option.h and option.c, http://lsec.cc.ac.cn/phg/), which was a parallel library for adaptive finite element methods and adaptive finite volume methods. It has been developed for more than 10 years and it has been applied in various areas.

# Purpose
The purpose of this project is to provide a standalone command line parsing framework under Linux, Unix and Mac such that any application can easily support command line options by using this library.

This project has refactorized original code, deleted some types, changed work flow, and added new types.

The following demo shows options for integer, floating point number, string, vector of integer, vector of floating point number and vector of strings, and details can be found in example/example.c:
```
./example -i 1 -f 1.2 -s "hi" -vi "1 2 3 5" -vf "1.3 2e-4 3.333" -vs "hi jill and jack"

```

# Build
The simplest way to install is to run commands:
```
 ./configure
 make
 sudo make install
```

The default destination is **/usr/local/flexop/**. However, the destination can be changed by using command:
```
 ./configure --prefix=DES_PATH
```

By this, the library will be installed to **DES_PATH**.


## Configure
The script **configure** has a few parameters:
```
./configure --help

Installation directories:
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [/usr/local/flexop]

By default, ake install' will install all the files in
usr/local/flexop/bin', usr/local/flexop/lib' etc.  You can specify
an installation prefix other than usr/local/flexop' using 

Optional Features:
  --disable-option-checking  ignore unrecognized --enable/--with options
  --disable-FEATURE       do not include FEATURE (same as --enable-FEATURE=no)
  --enable-FEATURE[=ARG]  include FEATURE [ARG=yes]
  --disable-assert        turn off assertions
  --enable-big-int        use long int for INT
  --disable-big-int       use int for INT (default),
  --with-int=type         integer type(long|long long)
  --enable-long-double    use long double for FLOAT
  --disable-long-double   use double for FLOAT (default)

```

The default integer type is **int** and the default floating point number is **double**. User can change integer and floating point number types, suchas **./configure --enable-big-int --with-int="long"** for **long int**, **./configure --enable-big-int --with-int="long long"** for **long long int**, **./configure --enable-long-double"** for **long double**.
