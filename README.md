# zenoh C Client API

## Building 
To build the **zenoh-c** client API you need to ensure that [cmake](https://cmake.org) is available on your platform -- if not please install it. 

Once the [cmake](https://cmake.org) dependency is satisfied, just do the following for **CMake** version 3 and higher:

  -- CMake version 3 and higher -- 

  ```bash
  $ cd /path/to/zenoh-c
  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_BUILD_TYPE=Release ..
  $ make 
  $ make install # on linux use **sudo**
  ```

For those that still have **CMake** version 2.8, do the following commands:

  -- CMake version 3 and higher -- 

  ```bash
  $ cd /path/to/zenoh-c
  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_BUILD_TYPE=Release ../cmake-2.8
  $ make 
  $ make install # on linux use **sudo**
  ```

If you want to build with debug symbols configure with the option -DCMAKE_BUILD_TYPE=Debug.
