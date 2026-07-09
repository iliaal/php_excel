--TEST--
excel.license_key / excel.license_name are PHP_INI_SYSTEM: ini_set() cannot mutate them at runtime
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!extension_loaded("excel") || ini_get("excel.license_key") === false) {
    print "skip license INI not registered (libxl without SetKey support)";
}
?>
--DESCRIPTION--
CR-014: the commercial license credentials were registered PHP_INI_ALL, so a
per-request ini_set() (or a per-directory .user.ini) could overwrite the key
in a shared pool. As PHP_INI_SYSTEM a runtime ini_set() must fail and leave
the value unchanged; a runtime key can still be supplied per book via the
ExcelBook constructor.
--FILE--
<?php
foreach (["excel.license_key", "excel.license_name"] as $ini) {
    $before = ini_get($ini);
    $ret = @ini_set($ini, "runtime-injected-value");
    $after = ini_get($ini);
    echo "$ini: ini_set returned " . var_export($ret, true) .
         ", unchanged=" . var_export($before === $after, true) . "\n";
}
echo "OK\n";
?>
--EXPECT--
excel.license_key: ini_set returned false, unchanged=true
excel.license_name: ini_set returned false, unchanged=true
OK
