Executor Context
================

Repository to contain prototype experimentation of Executor Context using 
hwloc and others.


Instructions
------------

1. Create a build directory and change to it

   $ mkdir build; cd build

2. Run cmake using the upper level as source

   $ cmake ../

3. Run ctest

   $ ctest

All tests should pass


Requirements
------------

Ubuntu Linux 16.04:

* hwloc 
* cmake 3.8 or above
* gcc 5.8 or above

Build system will try to run clang-tidy if found.

Contains
--------

* gsl headers, under MIT License
* CMake module for HWloc, under BSD license.
