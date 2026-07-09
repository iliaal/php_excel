--TEST--
AS_DATE packs float timestamps as dates; addPictureFromFile fails closed on open_basedir with a single warning
--EXTENSIONS--
excel
--FILE--
<?php
// CR-018: ExcelFormat::AS_DATE was only honored for integer values. A float
// unix timestamp was written as a bare number (isDate() == false).
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("D");
$s->write(3, 0, 1700000000,   null, ExcelFormat::AS_DATE);   // int
$s->write(3, 1, 1700000000.5, null, ExcelFormat::AS_DATE);   // float
$s->write(3, 2, 1700000000.5);                               // plain float, no AS_DATE
echo "int   isDate: "; var_dump($s->isDate(3, 0));
echo "float isDate: "; var_dump($s->isDate(3, 1));
echo "plain isDate: "; var_dump($s->isDate(3, 2));

// CR-006: a plain-path open_basedir denial in addPictureFromFile must fail
// closed (RETURN_FALSE) after a single warning, not fall through to the
// stream wrapper and warn a second time.
$warnings = [];
set_error_handler(function ($n, $str) use (&$warnings) { $warnings[] = $str; return true; });
ini_set("open_basedir", __DIR__);
$ret = (new ExcelBook())->addPictureFromFile("/etc/hostname");
ini_restore("open_basedir");
restore_error_handler();

echo "picture ret: "; var_dump($ret);
echo "warning count: " . count($warnings) . "\n";
echo "is open_basedir warning: " . var_export(
    count($warnings) === 1 && str_contains($warnings[0], "open_basedir"), true) . "\n";

echo "OK\n";
?>
--EXPECT--
int   isDate: bool(true)
float isDate: bool(true)
plain isDate: bool(false)
picture ret: bool(false)
warning count: 1
is open_basedir warning: true
OK
