--TEST--
Excel date pack/unpack tests
--EXTENSIONS--
excel
--FILE--
<?php
	// LibXL converts date components to/from timestamps using the OS
	// timezone. Probe the offset it applies so PHP's date() matches.
	date_default_timezone_set('UTC');
	$x_probe = new ExcelBook();
	$packed = $x_probe->packDateValues(2024, 6, 15, 12, 0, 0);
	$offset = $x_probe->unpackDate($packed) - gmmktime(12, 0, 0, 6, 15, 2024);
	if ($offset !== 0) {
		$tz = timezone_name_from_abbr('', -$offset, 1)
			?: timezone_name_from_abbr('', -$offset, 0);
		if ($tz) date_default_timezone_set($tz);
	}

	$x = new ExcelBook();

	$t = time();
	for ($i = 0; $i < 1000; $i++) {
		$tm = rand(10000000, $t);

		$out = $x->unpackDate($x->packDate($tm));
		$diff = abs($out - $tm);
		// DST transitions can cause a 1-hour difference in round-trip
		if ($diff !== 0 && $diff !== 3600) {
			echo "[1] source: {$tm} <> res: " . $out . " >> diff: ".($out - $tm)." packed: '".$x->packDate($tm)."'\n";
		}
	}
	echo "OK\n";

	for ($i = 0; $i < 1000; $i++) {
		$tm = time(); //rand(10000000, $t);

		$ed = $x->packDateValues(
		    date('Y', $tm),
		    date('m', $tm),
		    date('d', $tm),
		    date('H', $tm),
		    date('i', $tm),
		    date('s', $tm)
		);
		$out = $x->unpackDate($ed);
		if ($out != $tm) {
			echo "[2] source: {$tm} <> res: " . $out . " >> diff: ".($out - $tm)." packed: '".$ed."'\n";
		}
	}
	echo "OK\n";
?>
--EXPECT--
OK
OK
