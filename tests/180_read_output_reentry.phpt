--TEST--
ExcelSheet::read completes native work before destroying the format output value
--EXTENSIONS--
excel
--FILE--
<?php
class ReconstructBookOnDestruct
{
    public function __destruct()
    {
        global $book;
        $book->__construct(null, null, true);
    }
}

$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('S');
$sheet->write(1, 0, 'x');
$format = new ReconstructBookOnDestruct();

var_dump($sheet->read(1, 0, $format));
var_dump($format instanceof ExcelFormat);
var_dump(@$format->numberFormat());
echo "OK\n";
?>
--EXPECT--
string(1) "x"
bool(true)
bool(false)
OK
