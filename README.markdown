## Description

This extension uses libXl library to provide API for generating or parsing
all Excel files.

libxl is a high-performance mechanism for working with Excel files and is
able to generate output readable on Blackberries, iPhone, Office Products, 
Numbers, etc...

## Documentation

Please see the ```docs/``` and the ```tests/``` directory.

## Resources

* [libxl](http://www.libxl.com/)

## Installation

### Linux

    // change into php source files directory
    cd php-5.x.x
    
    // clone repository into php extension dir
    git clone https://github.com/iliaal/php_excel.git ext/excel
    
    // rebuild configure
    ./buildconf --force
    
    // replace <PATH> with the file path to the extracted libxl files
    // on a 32-bit platform use
    ./configure --with-excel --with-libxl-incdir=<PATH>/libxl-3.6.0.1/include_c --with-libxl-libdir=<PATH>/libxl-3.6.0.1/lib
    
    // on a 64-bit platform use
    ./configure --with-excel --with-libxl-incdir=<PATH>/libxl-3.6.0.1/include_c --with-libxl-libdir=<PATH>/libxl-3.6.0.1/lib64

### Windows

Pre-build packages for Windows can be downloaded [here](http://windows.php.net/downloads/pecl/snaps/excel).

### php.ini settings

You can save your libxl credentials in the php.ini to prevent unvealing them in your code. 

    [php_excel]
    excel.license_name="<YOUR_LICENSE_NAME>"
    excel.license_key="<YOUR_LICENSE_KEY>"
    excel.skip_empty=0