--TEST--
Write rejections name the offending reason, and bulk writers name the cell
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('S');

$messages = [];
set_error_handler(function ($no, $msg) use (&$messages) {
	$messages[] = $msg;
	return true;
});

function reason(callable $fn, array &$messages): string {
	$messages = [];
	$fn();
	return $messages ? preg_replace('/^\w+::\w+\(\): /', '', $messages[0]) : '(no warning)';
}

echo reason(fn() => $sheet->write(1, 0, NAN), $messages), "\n";
echo reason(fn() => $sheet->write(1, 0, "a\0b"), $messages), "\n";
echo reason(fn() => $sheet->write(1, 0, "text", null, ExcelFormat::AS_DATE), $messages), "\n";
echo reason(fn() => $sheet->writeRow(2, [1, 2, "a\0b"], 5), $messages), "\n";
echo reason(fn() => $sheet->writeCol(3, [1, NAN], 7), $messages), "\n";

restore_error_handler();

/* The rejected bulk writes must not have modified any cell. */
var_dump($sheet->read(2, 5), $sheet->read(2, 6));
echo "OK\n";
?>
--EXPECT--
Cell value cannot be written: number is not finite (NAN/INF)
Cell value cannot be written: string must not contain NUL bytes
Cell value cannot be written: value type does not match the requested data type
writeRow: value for column 7 cannot be written: string must not contain NUL bytes; no cells were modified
writeCol: value for row 8 cannot be written: number is not finite (NAN/INF); no cells were modified
string(0) ""
string(0) ""
OK
