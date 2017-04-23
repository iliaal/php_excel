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

cd php-7.x.x

# clone repository into php extension dir
git clone https://github.com/iliaal/php_excel.git -b php7 ext/excel

# rebuild configure
./buildconf --force

# replace <PATH> with the file path to the extracted libxl files
# on a 32-bit platform use
./configure --with-excel=shared --with-libxl-incdir=<PATH>/libxl-3.6.5.0/include_c --with-libxl-libdir=<PATH>/libxl-3.6.5.0/lib

# on a 64-bit platform use
./configure --with-excel=shared --with-libxl-incdir=<PATH>/libxl-3.6.5.0/include_c --with-libxl-libdir=<PATH>/libxl-3.6.5.0/lib64
```
 
### Windows

Pre-build packages for Windows can be downloaded [here](https://www.apachelounge.com/viewtopic.php?t=6617). To build the package for
Windows on your own you may want to visit this [project](https://github.com/johmue/win-php-sdk-builder).

### PHP7

* requires LibXL 3.6.0+
* use the php7 branch of the github repo

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

To prevent unvealing your credentials in your code you can save them in your php.ini file.
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
if you create an Excel sheet with php_excel and write a formula like ```=SUM(A1:B1)``` in cell ```C1``` you cannot
read the value of the calculation by reading cell ```C1``` in a later step. There have been observations that
this can also affect the OS pre-view of Excel files which rely on values. You can circumvent this by opening and saving the file directly
in Excel by using the [COM](http://de2.php.net/manual/en/class.com.php) or [DOTNET](http://de2.php.net/manual/en/class.dotnet.php)
interface. (**Excel is required!**)

``` php
/**
  * if you are having trouble try adding usleep(1000000) between the steps
  * for a pause of 1s or kill running Excel tasks beforehand e.g. with PHP on Windows
  * exec('TASKKILL /F /FI "IMAGENAME eq EXCEL.EXE" /T', $out);
  */

$workbook = realpath($file);
$excelHandler = new \COM("Excel.sheet") or die('Failed to connect Excel COM handler in file '.__FILE__.' on line '.__LINE__);
$excelHandler->Application->Workbooks->Open($workbook) or die('Failed to open Excel Workbook '.$file.' in file '.__FILE__.' on line '.__LINE__);
$excelHandler->Application->ActiveWorkbook->Save();
$excelHandler->Application->ActiveWorkbook->Close();
$excelHandler->Application->Quit();
$excelHandler = null;
```

### multibyte characters in credentials

If your credentials do not work properly because of multibyte characters you can extend ExcelBook
class and circumvent the build-in mechanism for ```php.ini``` settings.

``` php
<?php

class MyExcelBook extends \ExcelBook
{
    public function __construct($license_name=null, $license_key=null, $new_excel=false)
    {
        if (null === $license_name) {
            $license_name = utf8_decode(get_cfg_var('excel.license_name'));
            $license_key = utf8_decode(get_cfg_var('excel.license_key'));
        }
        parent::__construct($license_name, $license_key, $new_excel);
        $this->setLocale('UTF-8');
    }
}
```

### Testing (Windows)

After you have build the extension you can use ```nmake test``` to run the test suite against the php_excel extension.

``` shell
..\php-7.1.4> nmake test TESTS="-d extension=php_excel.dll -d date.timezone=\"America/Toronto\" -d excel.license_name=\"LICENSE_NAME\" -d excel.license_key=\"LICENSE_KEY\" ./ext/php_excel"
```

### Further reading

* [Performant-Handling-Excel-Files-PHP](http://blog.mayflower.de/4922-Performant-Handling-Excel-Files-PHP.html)
