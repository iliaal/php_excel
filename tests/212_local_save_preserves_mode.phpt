--TEST--
Atomic local save preserves the destination's permission bits
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (stripos(PHP_OS_FAMILY, 'win') !== false) print "skip POSIX permission bits required";
?>
--FILE--
<?php
$dir = __DIR__ . '/local_save_mode_' . getmypid();
mkdir($dir);
$dest = $dir . '/book.xlsx';

function mk(): ExcelBook {
	$book = new ExcelBook(null, null, true);
	$book->addSheet('S')->write(1, 0, 'payload');
	return $book;
}

/* Replacing an existing file must not widen its mode. */
file_put_contents($dest, 'placeholder');
chmod($dest, 0600);
var_dump(mk()->save($dest));
clearstatcache();
printf("mode: %04o\n", fileperms($dest) & 07777);

/* Staging must not leave temporary files behind. */
$leftover = array_values(array_filter(scandir($dir), fn($f) => str_ends_with($f, '.tmp')));
printf("leftover tmp: %d\n", count($leftover));

/* The saved workbook must still be readable. */
$check = new ExcelBook(null, null, true);
var_dump($check->loadFile($dest));
var_dump($check->getSheet(0)->read(1, 0));

/* A destination that does not exist yet just takes the umask default. */
$fresh = $dir . '/fresh.xlsx';
var_dump(mk()->save($fresh));
clearstatcache();
var_dump(file_exists($fresh) && filesize($fresh) > 0);

unlink($dest);
unlink($fresh);
rmdir($dir);
?>
--EXPECT--
bool(true)
mode: 0600
leftover tmp: 0
bool(true)
string(7) "payload"
bool(true)
bool(true)
