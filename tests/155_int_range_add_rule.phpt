--TEST--
libxl int/enum boundary on add/rule/constructor APIs (not just setters)
--EXTENSIONS--
excel
--SKIPIF--
<?php
/* Conditional-formatting rules need the 4-arg addConditionalFormatting
 * (libxl 5.1.0+); on 4.6.0 that method takes no arguments. */
if ((new ReflectionMethod("ExcelSheet", "addConditionalFormatting"))->getNumberOfParameters() < 4) print "skip libxl 5.1.0+ required";
?>
--DESCRIPTION--
The int-range guard must cover every public enum/type/color long that reaches a
libxl int parameter, including the add*/rule APIs and the ExcelTable
constructor -- not only the set* methods.
--FILE--
<?php

// Values above INT_MAX are floats on 32-bit PHP and fail ZPP before libxl.
function intOverflowResult(callable $call): bool {
    try {
        $result = $call();
    } catch (TypeError $e) {
        if (PHP_INT_SIZE !== 4) {
            throw $e;
        }
        return false;
    }
    if (PHP_INT_SIZE === 4 || $result !== false) {
        throw new UnexpectedValueException('Expected integer overflow rejection');
    }
    return $result;
}

$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$s->write(1, 0, "H");
$s->write(2, 0, "a");

echo "--- out of int range -> rejected ---\n";
echo "addDataValidation(huge op):  "; var_dump(intOverflowResult(fn() => @$s->addDataValidation(1, 2147483648, 2, 5, 1, 1, "10", "100")));
echo "addDataValidation(huge type):"; var_dump(intOverflowResult(fn() => @$s->addDataValidation(2147483648, 0, 2, 5, 1, 1, "10", "100")));
echo "addIgnoredError(huge):       "; var_dump(intOverflowResult(fn() => @$s->addIgnoredError(2147483648, 1, 0, 1, 0)));
echo "setProtect(huge enhanced):   "; var_dump(intOverflowResult(fn() => @$s->setProtect(true, "", 2147483648)));
echo "addTable(huge style):        "; var_dump(intOverflowResult(fn() => @$s->addTable("Tbad", 1, 2, 0, 0, true, 2147483648)));

try {
    new ExcelTable($s, "Tbad2", 1, 2, 0, 0, true, 2147483648);
    echo "new ExcelTable(huge style):  no throw (WRONG)\n";
} catch (\Throwable $e) {
    if (PHP_INT_SIZE === 4 && !($e instanceof TypeError)) {
        throw $e;
    }
    echo "new ExcelTable(huge style):  threw\n";
}

$cfing = $s->addConditionalFormatting(1, 5, 0, 0);
$cf = $b->addConditionalFormat();
echo "addRule(huge type):          "; var_dump(intOverflowResult(fn() => @$cfing->addRule(2147483648, $cf, "10")));
echo "addTopRule(huge value):      "; var_dump(intOverflowResult(fn() => @$cfing->addTopRule($cf, 2147483648, false, false)));
echo "addOpNumRule(huge op):       "; var_dump(intOverflowResult(fn() => @$cfing->addOpNumRule(2147483648, $cf, 1.0, 2.0)));
echo "add2ColorScaleRule(huge col):"; var_dump(intOverflowResult(fn() => @$cfing->add2ColorScaleRule(2147483648, 1, 0, 0.0, 0, 100.0)));
echo "add3ColorScaleRule(huge mid):"; var_dump(intOverflowResult(fn() => @$cfing->add3ColorScaleRule(1, 2147483648, 2, 0, 0.0, 0, 50.0, 0, 100.0)));

echo "--- valid values accepted ---\n";
echo "addDataValidation:           "; var_dump($s->addDataValidation(1, 0, 2, 5, 1, 1, "10", "100"));
echo "addIgnoredError:             "; var_dump($s->addIgnoredError(1, 1, 0, 1, 0));
echo "addRule:                     "; var_dump($cfing->addRule(0x100, $cf, "10"));
echo "addTable instanceof:         "; var_dump($s->addTable("Tok", 1, 2, 0, 0, true, 0) instanceof ExcelTable);
echo "new ExcelTable instanceof:   "; var_dump((new ExcelTable($s, "Tok2", 1, 2, 0, 0, true, 0)) instanceof ExcelTable);

echo "OK\n";
?>
--EXPECT--
--- out of int range -> rejected ---
addDataValidation(huge op):  bool(false)
addDataValidation(huge type):bool(false)
addIgnoredError(huge):       bool(false)
setProtect(huge enhanced):   bool(false)
addTable(huge style):        bool(false)
new ExcelTable(huge style):  threw
addRule(huge type):          bool(false)
addTopRule(huge value):      bool(false)
addOpNumRule(huge op):       bool(false)
add2ColorScaleRule(huge col):bool(false)
add3ColorScaleRule(huge mid):bool(false)
--- valid values accepted ---
addDataValidation:           bool(true)
addIgnoredError:             bool(true)
addRule:                     bool(true)
addTable instanceof:         bool(true)
new ExcelTable instanceof:   bool(true)
OK
