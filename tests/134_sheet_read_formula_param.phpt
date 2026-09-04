--TEST--
ExcelSheet::read/readRow/readCol with read_formula parameter
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet("Formulas");

$sheet->write(1, 0, "=2+3");
$sheet->write(1, 1, "=10*2");
$sheet->write(1, 2, "hello");
$sheet->write(2, 0, "=100+1");
$sheet->write(3, 0, "=200+2");

$tmp = tempnam(sys_get_temp_dir(), "xl_formula_") . ".xlsx";
$book->save($tmp);

$book2 = new ExcelBook(null, null, true);
$book2->loadFile($tmp);
$sheet2 = $book2->getSheet(0);

// read() with read_formula=true (default) returns the formula string
$fmt = null;
$formula = $sheet2->read(1, 0, $fmt, true);
echo "read(formula=true): ";
var_dump($formula);

// read() with read_formula=false returns the cached numeric value
$fmt = null;
$value = $sheet2->read(1, 0, $fmt, false);
echo "read(formula=false): ";
var_dump($value);

// read() on a non-formula cell is unaffected by read_formula
$fmt = null;
$plain = $sheet2->read(1, 2, $fmt, false);
echo "read(plain, formula=false): ";
var_dump($plain);

// readRow() with read_formula=true
$row_formulas = $sheet2->readRow(1, 0, 1, true);
echo "readRow(formula=true): ";
var_dump($row_formulas);

// readRow() with read_formula=false
$row_values = $sheet2->readRow(1, 0, 1, false);
echo "readRow(formula=false): ";
var_dump($row_values);

// readCol() with read_formula=true
$col_formulas = $sheet2->readCol(0, 1, 3, true);
echo "readCol(formula=true): ";
var_dump($col_formulas);

// readCol() with read_formula=false
$col_values = $sheet2->readCol(0, 1, 3, false);
echo "readCol(formula=false): ";
var_dump($col_values);

unlink($tmp);
echo "OK\n";
?>
--EXPECT--
read(formula=true): string(3) "2+3"
read(formula=false): float(0)
read(plain, formula=false): string(5) "hello"
readRow(formula=true): array(2) {
  [0]=>
  string(3) "2+3"
  [1]=>
  string(4) "10*2"
}
readRow(formula=false): array(2) {
  [0]=>
  float(0)
  [1]=>
  float(0)
}
readCol(formula=true): array(3) {
  [0]=>
  string(3) "2+3"
  [1]=>
  string(5) "100+1"
  [2]=>
  string(5) "200+2"
}
readCol(formula=false): array(3) {
  [0]=>
  float(0)
  [1]=>
  float(0)
  [2]=>
  float(0)
}
OK
