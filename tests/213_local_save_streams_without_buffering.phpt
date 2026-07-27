--TEST--
Atomic local save streams through LibXL instead of buffering the workbook in PHP memory
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!ExcelBook::requiresKey() || !ini_get("excel.license_name") || !ini_get("excel.license_key")) print "skip needs a license for a workbook large enough to measure";
?>
--FILE--
<?php
$dir = __DIR__ . '/local_save_stream_' . getmypid();
mkdir($dir);
$dest = $dir . '/big.xlsx';

$book = new ExcelBook(ini_get('excel.license_name'), ini_get('excel.license_key'), true);
$sheet = $book->addSheet('S');
for ($row = 1; $row <= 4000; $row++) {
	$values = [];
	for ($col = 0; $col < 20; $col++) {
		$values[] = ($col % 4 === 0) ? "value-string-$row-$col" : ($row * 1000 + $col);
	}
	$sheet->writeRow($row, $values);
}

/* Saving to a path must not allocate a PHP-side copy of the archive, so the
 * request's peak allocation must not grow by anything like the file size. */
$before = memory_get_peak_usage(true);
var_dump($book->save($dest));
$growth = memory_get_peak_usage(true) - $before;

clearstatcache();
$size = filesize($dest);
var_dump($size > 200000);
var_dump($growth < $size);

/* save() with no path returns the archive, so that one does allocate. */
$raw = $book->save();
var_dump(strlen($raw) > 200000);

unlink($dest);
rmdir($dir);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
