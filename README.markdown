Description
-----------
This extension uses libXl library to provide API for generating or parsing
all Excel files.

libxl is a high-performance mechanism for working with Excel files and is
able to generate output readable on Blackberries, iPhone, Office Products, 
Numbers, etc...

Documentation
-------------
Please see the ```docs/``` and the ```tests/``` directory.

Resources
---------
* [libxl](http://www.libxl.com/)

Installation
------------
**Linux**
    // please replace <PATH> with the file path where you have extracted libxl
    // 32-bit
    ./configure --with-excel=<PATH>/libxl-3.6.0.1/ --with-libxl-incdir=<PATH>/libxl-3.6.0.1/include_c --with-libxl-libdir=<PATH>/libxl-3.6.0.1/lib
    
    // 64-bit
    ./configure --with-excel=<PATH>/libxl-3.6.0.1/ --with-libxl-incdir=<PATH>/libxl-3.6.0.1/include_c --with-libxl-libdir=<PATH>/libxl-3.6.0.1/lib64

**Windows**
Pre-build packages for Windows can be downloaded [here](http://windows.php.net/downloads/pecl/snaps/excel).