--TEST--
libxl int/RGB setters reject out-of-range values instead of silently truncating
--EXTENSIONS--
excel
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

echo "--- out of int range -> rejected ---\n";
echo "setZoom(2147483648):       "; var_dump(intOverflowResult(fn() => @$s->setZoom(2147483648)));
echo "setZoom(-1):                "; var_dump(@$s->setZoom(-1));
echo "setZoomPrint(2147483648):  "; var_dump(intOverflowResult(fn() => @$s->setZoomPrint(2147483648)));
echo "setPaper(2147483648):      "; var_dump(intOverflowResult(fn() => @$s->setPaper(2147483648)));
echo "setPrintFit(2147483648,1): "; var_dump(intOverflowResult(fn() => @$s->setPrintFit(2147483648, 1)));
echo "setPrintFit(1,-1):          "; var_dump(@$s->setPrintFit(1, -1));
echo "setBorder(...,2147483648): "; var_dump(intOverflowResult(fn() => @$s->setBorder(1, 2, 0, 1, 2147483648, 0)));
echo "setBorder(...,-1 color):    "; var_dump(@$s->setBorder(1, 2, 0, 1, 0, -1));

echo "--- RGB component out of 0-255 -> rejected ---\n";
echo "setTabRgbColor(256,0,0):    "; var_dump(@$s->setTabRgbColor(256, 0, 0));
echo "setTabRgbColor(0,-1,0):     "; var_dump(@$s->setTabRgbColor(0, -1, 0));
echo "setTabRgbColor(0,0,PHP_INT_MAX): "; var_dump(@$s->setTabRgbColor(0, 0, PHP_INT_MAX));

echo "--- pixel dimensions and other Sheet/Book setters -> rejected ---\n";
echo "setColPx(0,0,2147483648):  "; var_dump(intOverflowResult(fn() => @$s->setColPx(0, 0, 2147483648)));
echo "setRowPx(1,2147483648):    "; var_dump(intOverflowResult(fn() => @$s->setRowPx(1, 2147483648)));
echo "setTabColor(2147483648):   "; var_dump(intOverflowResult(fn() => @$s->setTabColor(2147483648)));
echo "setRightToLeft(2147483648):"; var_dump(intOverflowResult(fn() => @$s->setRightToLeft(2147483648)));
echo "writeComment(huge width):   "; var_dump(intOverflowResult(fn() => @$s->writeComment(1, 0, "x", "a", 2147483648, 10)));
echo "setCalcMode(2147483648):   "; var_dump(intOverflowResult(fn() => @$b->setCalcMode(2147483648)));
echo "setDefaultFont(huge size):  "; var_dump(intOverflowResult(fn() => @$b->setDefaultFont("Arial", 2147483648)));

echo "--- ExcelBook int boundaries -> rejected ---\n";
echo "colorUnpack(4294967296):    "; var_dump(intOverflowResult(fn() => @$b->colorUnpack(4294967296)));
echo "colorUnpack(2147483648):   "; var_dump(intOverflowResult(fn() => @$b->colorUnpack(2147483648)));
echo "addFormatFromStyle(2**32):  "; var_dump(intOverflowResult(fn() => @$b->addFormatFromStyle(4294967296)));
echo "packDateValues(huge year):  "; var_dump(intOverflowResult(fn() => @$b->packDateValues(2147483648, 1, 1, 0, 0, 0)));
$cfid = $b->addCustomFormat("0.000");
echo "getCustomFormat(2**32+id):  "; var_dump(intOverflowResult(fn() => @$b->getCustomFormat(4294967296 + $cfid)));

echo "--- named-range scope aliasing -> rejected, sentinels preserved ---\n";
$s->setNamedRange("rng", 1, 3, 0, 2); // name, row, to_row, col, to_col
echo "getNamedRange(huge scope):  "; var_dump(intOverflowResult(fn() => @$s->getNamedRange("rng", 4294967295)));
echo "setNamedRange(huge scope):  "; var_dump(intOverflowResult(fn() => @$s->setNamedRange("r2", 1, 3, 0, 2, 4294967296)));
echo "delNamedRange(huge scope):  "; var_dump(intOverflowResult(fn() => @$s->delNamedRange("rng", 4294967296)));
echo "getNamedRange(default):     "; var_dump(is_array(@$s->getNamedRange("rng")));
echo "getNamedRange(SCOPE_WORKBOOK):"; var_dump(is_array(@$s->getNamedRange("rng", ExcelBook::SCOPE_WORKBOOK)));

echo "--- ConditionalFormat / Table setters -> rejected ---\n";
$cf = $b->addConditionalFormat();
echo "CF setBorder(2147483648):  "; var_dump(intOverflowResult(fn() => @$cf->setBorder(2147483648)));
echo "CF setBorderColor(-1):      "; var_dump(@$cf->setBorderColor(-1));
echo "CF setNumFormat(2147483648):"; var_dump(intOverflowResult(fn() => @$cf->setNumFormat(2147483648)));
echo "CF setFillPattern(2147483648):"; var_dump(intOverflowResult(fn() => @$cf->setFillPattern(2147483648)));
$t = new ExcelTable($s, "T", 1, 3, 0, 1, true, 0);
echo "Table setStyle(2147483648):"; var_dump(intOverflowResult(fn() => @$t->setStyle(2147483648)));

echo "--- valid values accepted ---\n";
echo "setZoom(120):               "; var_dump($s->setZoom(120));
echo "setPrintFit(1,1):           "; var_dump($s->setPrintFit(1, 1));
echo "setTabRgbColor(10,20,30):   "; var_dump($s->setTabRgbColor(10, 20, 30));
echo "setBorder(1,2,0,1,1,0):     "; var_dump($s->setBorder(1, 2, 0, 1, 1, 0));
echo "setColPx(0,0,64):           "; var_dump($s->setColPx(0, 0, 64));
echo "setTabColor(1):             "; var_dump($s->setTabColor(1));
echo "setCalcMode(0):             "; var_dump($b->setCalcMode(0));
echo "CF setBorder(1):            "; var_dump($cf->setBorder(1));
echo "Table setStyle(1):          "; var_dump($t->setStyle(1));
echo "addFormatFromStyle(0):      "; var_dump($b->addFormatFromStyle(0) instanceof ExcelFormat);
echo "packDateValues(2024):       "; var_dump(is_float($b->packDateValues(2024, 6, 1, 12, 0, 0)));

echo "OK\n";
?>
--EXPECT--
--- out of int range -> rejected ---
setZoom(2147483648):       bool(false)
setZoom(-1):                bool(false)
setZoomPrint(2147483648):  bool(false)
setPaper(2147483648):      bool(false)
setPrintFit(2147483648,1): bool(false)
setPrintFit(1,-1):          bool(false)
setBorder(...,2147483648): bool(false)
setBorder(...,-1 color):    bool(false)
--- RGB component out of 0-255 -> rejected ---
setTabRgbColor(256,0,0):    bool(false)
setTabRgbColor(0,-1,0):     bool(false)
setTabRgbColor(0,0,PHP_INT_MAX): bool(false)
--- pixel dimensions and other Sheet/Book setters -> rejected ---
setColPx(0,0,2147483648):  bool(false)
setRowPx(1,2147483648):    bool(false)
setTabColor(2147483648):   bool(false)
setRightToLeft(2147483648):bool(false)
writeComment(huge width):   bool(false)
setCalcMode(2147483648):   bool(false)
setDefaultFont(huge size):  bool(false)
--- ExcelBook int boundaries -> rejected ---
colorUnpack(4294967296):    bool(false)
colorUnpack(2147483648):   bool(false)
addFormatFromStyle(2**32):  bool(false)
packDateValues(huge year):  bool(false)
getCustomFormat(2**32+id):  bool(false)
--- named-range scope aliasing -> rejected, sentinels preserved ---
getNamedRange(huge scope):  bool(false)
setNamedRange(huge scope):  bool(false)
delNamedRange(huge scope):  bool(false)
getNamedRange(default):     bool(true)
getNamedRange(SCOPE_WORKBOOK):bool(true)
--- ConditionalFormat / Table setters -> rejected ---
CF setBorder(2147483648):  bool(false)
CF setBorderColor(-1):      bool(false)
CF setNumFormat(2147483648):bool(false)
CF setFillPattern(2147483648):bool(false)
Table setStyle(2147483648):bool(false)
--- valid values accepted ---
setZoom(120):               NULL
setPrintFit(1,1):           bool(true)
setTabRgbColor(10,20,30):   bool(true)
setBorder(1,2,0,1,1,0):     bool(true)
setColPx(0,0,64):           bool(true)
setTabColor(1):             bool(true)
setCalcMode(0):             bool(true)
CF setBorder(1):            bool(true)
Table setStyle(1):          bool(true)
addFormatFromStyle(0):      bool(true)
packDateValues(2024):       bool(true)
OK
