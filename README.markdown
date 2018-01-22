## Description

This extension uses libXl library to provide API for generating or parsing
all Excel files.

libxl is a high-performance mechanism for working with Excel files and is
able to generate output readable on Blackberries, iPhone, Office Products, 
Numbers, etc...

## Documentation

Please see the ```docs/``` and the ```tests/``` directory.

## Resources

* [required LibXL library (commercial)](http://www.libxl.com/)
* [slides of confoo talk about php_excel](http://ilia.ws/files/confoo_phpexcel.pdf)

## Installation

### Linux

``` shell
# change into php source files directory
cd php-5.x.x

# clone repository into php extension dir
git clone https://github.com/iliaal/php_excel.git ext/excel

# to build php7 module, you should use php7 git branch
cd ext/excel && git checkout php7 && cd ../..

# rebuild configure
./buildconf --force

# replace <PATH> with the file path to the extracted libxl files
# on a 32-bit platform use
./configure --with-excel --with-libxl-incdir=<PATH>/libxl-3.6.0.1/include_c --with-libxl-libdir=<PATH>/libxl-3.6.0.1/lib

# on a 64-bit platform use
./configure --with-excel --with-libxl-incdir=<PATH>/libxl-3.6.0.1/include_c --with-libxl-libdir=<PATH>/libxl-3.6.0.1/lib64
```
 
### Windows

Pre-build packages for Windows can be downloaded [here](http://windows.php.net/downloads/pecl/snaps/excel).

## Getting started

``` php
<?php
    
// init excel work book as xlsx
$useXlsxFormat = true;
$xlBook = new \ExcelBook('<YOUR_LICENSE_NAME>', '<YOUR_LICENSE_KEY>', $useXlsxFormat);
$xlBook->setLocale('UTF-8');

// add sheet to work book
$xlSheet1 = $xlBook->addSheet('Sheet1');

// create a small sample data set
$dataset = [
    [1, 1500, 'John', 'Doe'],
    [2,  750, 'Jane', 'Doe']
];

// write data set to sheet
$row = 1;
foreach($dataset as $item){
    $xlSheet1->writeRow($row, $item);
    $row++;
}

// write sum formula under data set
$col = 1;
$xlSheet1->write($row, $col, '=SUM(B1:B3)');

// add second sheet to work book
$xlSheet2 = $xlBook->addSheet('Sheet2');

// add a date with specific date format to second sheet
$row = 1; $col = 0;
$date = new \DateTime('2014-08-02');
$dateFormat = new \ExcelFormat($xlBook);
$dateFormat->numberFormat(\ExcelFormat::NUMFORMAT_DATE);
$xlSheet2->write($row, $col, $date->getTimestamp(), $dateFormat, \ExcelFormat::AS_DATE);

// save workbook
$xlBook->save('test.xlsx');
```

## optional php.ini settings

To prevent unveiling your credentials in your code you can save them in your php.ini file.
They will be automatically fetched by the extension and you can pass ```null``` instead of
your credentials ```new \ExcelBook(null, null, $useXlsxFormat)```.

``` ini
; optional settings for excel extension
[excel]
excel.license_name="<YOUR_LICENSE_NAME>"
excel.license_key="<YOUR_LICENSE_KEY>"
excel.skip_empty=0
```

## Known Issues

### Formulas written but no values readable

**Excel stores value and formula** for each cell while **LibXL stores only the formula**. This means
if you create an Excel sheet with php_excel and write a formula like ```=SUM(A1:B1)``` in cell ```C1``` you can't
read the value of the calculation by reading cell ```C1``` in a later step. There has been observations that
this can also affect the OS pre-view of Excel files. You can circumvent this by opening and saving the file directly
in Excel or using the COM classes to open and save the Excel file via PHP. (**In both cases Excel is required!**)

### multibyte characters in credentials

If your credentials does not work properly because of multibyte characters you can compile php_excel with
```--with-xml --with-libxml --with-iconv``` and your credentials will be automatically ```utf8_decoded()```
before using with LibXL.

If you compile php_excel as a shared extension on Linux you need to provide the path to the libxml directory.
e.g. on Ubuntu you need to compile with ```--with-libxml-dir=/usr/include/libxml2```.

### Further reading

* [Performant-Handling-Excel-Files-PHP](http://blog.mayflower.de/4922-Performant-Handling-Excel-Files-PHP.html)
