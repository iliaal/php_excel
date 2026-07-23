--TEST--
CR-009: explicit dtype mismatches are rejected instead of silent wrong-type writes
--EXTENSIONS--
excel
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$warn = false;
set_error_handler(function ($n, $m) use (&$warn) {
	if (str_contains($m, 'cannot be written') || str_contains($m, 'Failed to write')) {
		$warn = true;
	}
	return true;
});

function run($label, $fn, &$warn) {
	$warn = false;
	$ret = $fn();
	echo $label . ": " . ($warn ? "reject " : "") . var_export($ret, true) . "\n";
}

run("str+AS_DATE", fn() => $s->write(1, 0, "not-a-date", null, ExcelFormat::AS_DATE), $warn);
run("int+AS_FORMULA", fn() => $s->write(1, 1, 42, null, ExcelFormat::AS_FORMULA), $warn);
run("bool+AS_TEXT", fn() => $s->write(1, 2, true, null, ExcelFormat::AS_TEXT), $warn);
run("null+AS_NUMERIC_STRING", fn() => $s->write(1, 3, null, null, ExcelFormat::AS_NUMERIC_STRING), $warn);
run("int+AS_DATE", fn() => $s->write(2, 0, time(), null, ExcelFormat::AS_DATE), $warn);
run("str+AS_TEXT", fn() => $s->write(2, 1, "hello", null, ExcelFormat::AS_TEXT), $warn);
run("str+AS_FORMULA", fn() => $s->write(2, 2, "A1+1", null, ExcelFormat::AS_FORMULA), $warn);
run("str+default", fn() => $s->write(2, 3, "=B1"), $warn);
run("writeRow mismatch", fn() => $s->writeRow(3, ["x", 1], 0, null, ExcelFormat::AS_DATE), $warn);
run("writeRow ok", fn() => $s->writeRow(4, [time(), time()], 0, null, ExcelFormat::AS_DATE), $warn);

restore_error_handler();
echo "OK\n";
?>
--EXPECT--
str+AS_DATE: reject false
int+AS_FORMULA: reject false
bool+AS_TEXT: reject false
null+AS_NUMERIC_STRING: reject false
int+AS_DATE: true
str+AS_TEXT: true
str+AS_FORMULA: true
str+default: true
writeRow mismatch: reject false
writeRow ok: true
OK
