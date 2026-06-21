--TEST--
Review fixes: error-cell format out-param, packDate epoch, single open_basedir warning
--EXTENSIONS--
excel
--FILE--
<?php
// Error cells must populate the format out-param like every other cell
// type. Previously CELLTYPE_ERROR left *format NULL, so read() built an
// ExcelFormat wrapper with a NULL handle that warned on every use.
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$s->writeError(1, 0, ExcelSheet::ERRORTYPE_DIV_0);
$raw = $b->save();

$b2 = new ExcelBook(null, null, true);
$b2->load($raw);
$s2 = $b2->getSheet(0);

$val = $s2->read(1, 0, $fmt);
echo "error cell value: "; var_dump($val);
echo "fmt is ExcelFormat: "; var_dump($fmt instanceof ExcelFormat);

$warned = 0;
set_error_handler(function () use (&$warned) { $warned++; });
$nf = $fmt->numberFormat();
restore_error_handler();
echo "fmt usable without warning: "; var_dump(is_int($nf) && $warned === 0);

// packDate must accept the Unix epoch (ts=0 -> 1970-01-01, a valid date)
// and reject only negative timestamps.
echo "packDate(0) is float: "; var_dump(is_float($b->packDate(0)));
echo "packDate(-1): "; var_dump($b->packDate(-1));

// A plain path denied by open_basedir must fail with a single warning,
// not fall through to the stream wrapper and warn a second time.
$count = 0;
set_error_handler(function () use (&$count) { $count++; });
ini_set("open_basedir", __DIR__);
$r = $b->loadFile("/etc/hostname");
ini_restore("open_basedir");
restore_error_handler();
echo "loadFile denied result: "; var_dump($r);
echo "open_basedir warning count: "; var_dump($count);
?>
--EXPECT--
error cell value: int(7)
fmt is ExcelFormat: bool(true)
fmt usable without warning: bool(true)
packDate(0) is float: bool(true)
packDate(-1): bool(false)
loadFile denied result: bool(false)
open_basedir warning count: int(1)
