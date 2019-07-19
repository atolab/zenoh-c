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

## Running the Examples
The simplest way to run some of the example is to get a prebuilt instance of the **zenoh** network 
router at [http://github.com/atolab/atobin](http://github.com/atolab/atobin) and then to run the 
examples on your machine.

### Starting the zenoh Network Service
Assuming you've downloaded the **zenoh** network service on the current directory, then simply do:

  $ ./zenoh 

To see the zenoh manual page, simply do:

  $ ./zenoh --help



### Basic Pub/Sub Example
Assuming that (1) you are running the **zenoh** network service,  and (2) you are under the build directory, do:

    $ ./z_sub

And on another shell, do:

    $ ./z_pub

## Storage and Query Example
Assuming you are running the **zenoh** network service, do:

    $ ./z_storage

And on another shell, do:

    $ ./z_pub

After a few publications just terminate the publisher, and then try to query the storage:

    $ ./z_query








