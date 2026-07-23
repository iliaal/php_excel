--TEST--
CR-006: plain local save stages via SaveRaw and preserves destination on failure
--EXTENSIONS--
excel
--FILE--
<?php
// Local path save no longer calls xlBookSave in place; it uses SaveRaw +
// stream staging. Verify a normal local save round-trips.
$dir = sys_get_temp_dir();
$dest = $dir . "/211_local_save_" . getmypid() . ".xlsx";
@unlink($dest);

$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$s->write(1, 0, "payload");
$ok = $b->save($dest);
echo "save: ";
var_dump($ok);
echo "exists: ";
var_dump(is_file($dest) && filesize($dest) > 0);

$b2 = new ExcelBook(null, null, true);
$ok2 = $b2->loadFile($dest);
echo "reload: ";
var_dump($ok2);
$s2 = $b2->getSheet(0);
echo "read: ";
var_dump($s2 ? $s2->read(1, 0) : false);

// No leftover staging temps
$temps = glob($dest . ".*.tmp") ?: [];
echo "temps: " . count($temps) . "\n";

@unlink($dest);
foreach ($temps as $t) {
	@unlink($t);
}
echo "OK\n";
?>
--EXPECT--
save: bool(true)
exists: bool(true)
reload: bool(true)
read: string(7) "payload"
temps: 0
OK
