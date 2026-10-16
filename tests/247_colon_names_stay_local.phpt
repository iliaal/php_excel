--TEST--
File names with a colon but no scheme save and load as plain local files
--EXTENSIONS--
excel
--SKIPIF--
<?php if (PHP_OS_FAMILY === "Windows") die("skip ':' is a drive or stream separator in Windows file names"); ?>
--FILE--
<?php
$warnings = [];
set_error_handler(function ($no, $message) use (&$warnings) {
    $warnings[] = $message;
    return true;
});

$absolute = sys_get_temp_dir() . '/report:2026-excel-247.xlsx';
foreach ([$absolute, 'report:2026-excel-247.xlsx', 'C:excel-247.xlsx'] as $name) {
    $warnings = [];
    $book = new ExcelBook(null, null, true);
    $book->addSheet('S')->write(1, 0, 'kept');
    $ok = $book->save($name);
    $onDisk = $name[0] === '/' ? $name : getcwd() . '/' . $name;
    clearstatcache();
    $size = is_file($onDisk) ? filesize($onDisk) : 0;
    $loaded = new ExcelBook(null, null, true);
    $read = $loaded->loadFile($name) ? $loaded->getSheet(0)->read(1, 0) : 'unloadable';
    @unlink($onDisk);
    echo ($name === $absolute ? 'absolute' : $name), ': ', var_export($ok, true), ' ', $size > 0 ? 'nonempty' : 'empty', " $read ", count($warnings), "\n";
}
restore_error_handler();
?>
--CLEAN--
<?php
foreach (scandir(getcwd()) as $f) {
    if (str_contains($f, 'excel-247')) {
        @unlink(getcwd() . '/' . $f);
    }
}
@unlink(sys_get_temp_dir() . '/report:2026-excel-247.xlsx');
?>
--EXPECT--
absolute: true nonempty kept 0
report:2026-excel-247.xlsx: true nonempty kept 0
C:excel-247.xlsx: true nonempty kept 0
