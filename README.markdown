## Description

This extension uses libXl library to provide API for generating or parsing
all Excel files.

libxl is a high-performance mechanism for working with Excel files and is
able to generate output readable on Blackberries, iPhone, Office Products, 
Numbers, etc...

## Documentation

Please see the ```docs/``` and the ```tests/``` directory.

## Resources

* [required libxl library](http://www.libxl.com/)
* [slides of confoo talk about php_excel](http://ilia.ws/files/confoo_phpexcel.pdf)

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

## Getting started

    <?php
    
    // init excel work book
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
    $xlSheet1->write($row, $col, '=SUM(B1:B3)', null, \ExcelFormat::AS_FORMULA);

    // add second sheet to work book
    $xlSheet2 = $xlBook->addSheet('Sheet2');

    // add a date with date format to sheet 2
    $row = 1;
    $col = 0;
    // convert date to excel date (=number of days since 01.01.1900)
    $date = new \DateTime('2014-08-02');
    $date = $xlBook->packDate($date->getTimestamp());
    // create date format otherwise you will see the number of days instead of a date in the cell
    $format = new \ExcelFormat($xlBook);
    $format->numberFormat(\ExcelFormat::NUMFORMAT_DATE);
    // write date to second sheet
    $xlSheet2->write($row, $col, $date, $format, \ExcelFormat::AS_DATE);

    // save workbook
    $xlBook->save('test.xlsx');

## optional php.ini settings

To prevent unvealing your credentials in your code you can save them in your php.ini file.
They will be automatically fetched by the extension and you can pass ```null``` instead of
your credentials ```new \ExcelBook(null, null, $useXlsxFormat)```.

    ; optional settings for excel extension
    [excel]
    excel.license_name="<YOUR_LICENSE_NAME>"
    excel.license_key="<YOUR_LICENSE_KEY>"
    excel.skip_empty=0