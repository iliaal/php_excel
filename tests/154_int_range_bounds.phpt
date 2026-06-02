--TEST--
libxl int/RGB setters reject out-of-range values instead of silently truncating
--EXTENSIONS--
excel
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");

echo "--- out of int range -> rejected ---\n";
echo "setZoom(PHP_INT_MAX):       "; var_dump(@$s->setZoom(PHP_INT_MAX));
echo "setZoom(-1):                "; var_dump(@$s->setZoom(-1));
echo "setZoomPrint(PHP_INT_MAX):  "; var_dump(@$s->setZoomPrint(PHP_INT_MAX));
echo "setPaper(PHP_INT_MAX):      "; var_dump(@$s->setPaper(PHP_INT_MAX));
echo "setPrintFit(PHP_INT_MAX,1): "; var_dump(@$s->setPrintFit(PHP_INT_MAX, 1));
echo "setPrintFit(1,-1):          "; var_dump(@$s->setPrintFit(1, -1));
echo "setBorder(...,PHP_INT_MAX): "; var_dump(@$s->setBorder(1, 2, 0, 1, PHP_INT_MAX, 0));
echo "setBorder(...,-1 color):    "; var_dump(@$s->setBorder(1, 2, 0, 1, 0, -1));

echo "--- RGB component out of 0-255 -> rejected ---\n";
echo "setTabRgbColor(256,0,0):    "; var_dump(@$s->setTabRgbColor(256, 0, 0));
echo "setTabRgbColor(0,-1,0):     "; var_dump(@$s->setTabRgbColor(0, -1, 0));
echo "setTabRgbColor(0,0,PHP_INT_MAX): "; var_dump(@$s->setTabRgbColor(0, 0, PHP_INT_MAX));

echo "--- pixel dimensions and other Sheet/Book setters -> rejected ---\n";
echo "setColPx(0,0,PHP_INT_MAX):  "; var_dump(@$s->setColPx(0, 0, PHP_INT_MAX));
echo "setRowPx(1,PHP_INT_MAX):    "; var_dump(@$s->setRowPx(1, PHP_INT_MAX));
echo "setTabColor(PHP_INT_MAX):   "; var_dump(@$s->setTabColor(PHP_INT_MAX));
echo "setRightToLeft(PHP_INT_MAX):"; var_dump(@$s->setRightToLeft(PHP_INT_MAX));
echo "writeComment(huge width):   "; var_dump(@$s->writeComment(1, 0, "x", "a", PHP_INT_MAX, 10));
echo "setCalcMode(PHP_INT_MAX):   "; var_dump(@$b->setCalcMode(PHP_INT_MAX));
echo "setDefaultFont(huge size):  "; var_dump(@$b->setDefaultFont("Arial", PHP_INT_MAX));

echo "--- ExcelBook int boundaries -> rejected ---\n";
echo "colorUnpack(4294967296):    "; var_dump(@$b->colorUnpack(4294967296));
echo "colorUnpack(PHP_INT_MAX):   "; var_dump(@$b->colorUnpack(PHP_INT_MAX));
echo "addFormatFromStyle(2**32):  "; var_dump(@$b->addFormatFromStyle(4294967296));
echo "packDateValues(huge year):  "; var_dump(@$b->packDateValues(PHP_INT_MAX, 1, 1, 0, 0, 0));

echo "--- ConditionalFormat / Table setters -> rejected ---\n";
$cf = $b->addConditionalFormat();
echo "CF setBorder(PHP_INT_MAX):  "; var_dump(@$cf->setBorder(PHP_INT_MAX));
echo "CF setBorderColor(-1):      "; var_dump(@$cf->setBorderColor(-1));
echo "CF setNumFormat(PHP_INT_MAX):"; var_dump(@$cf->setNumFormat(PHP_INT_MAX));
echo "CF setFillPattern(PHP_INT_MAX):"; var_dump(@$cf->setFillPattern(PHP_INT_MAX));
$t = new ExcelTable($s, "T", 1, 3, 0, 1, true, 0);
echo "Table setStyle(PHP_INT_MAX):"; var_dump(@$t->setStyle(PHP_INT_MAX));

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
setZoom(PHP_INT_MAX):       bool(false)
setZoom(-1):                bool(false)
setZoomPrint(PHP_INT_MAX):  bool(false)
setPaper(PHP_INT_MAX):      bool(false)
setPrintFit(PHP_INT_MAX,1): bool(false)
setPrintFit(1,-1):          bool(false)
setBorder(...,PHP_INT_MAX): bool(false)
setBorder(...,-1 color):    bool(false)
--- RGB component out of 0-255 -> rejected ---
setTabRgbColor(256,0,0):    bool(false)
setTabRgbColor(0,-1,0):     bool(false)
setTabRgbColor(0,0,PHP_INT_MAX): bool(false)
--- pixel dimensions and other Sheet/Book setters -> rejected ---
setColPx(0,0,PHP_INT_MAX):  bool(false)
setRowPx(1,PHP_INT_MAX):    bool(false)
setTabColor(PHP_INT_MAX):   bool(false)
setRightToLeft(PHP_INT_MAX):bool(false)
writeComment(huge width):   bool(false)
setCalcMode(PHP_INT_MAX):   bool(false)
setDefaultFont(huge size):  bool(false)
--- ExcelBook int boundaries -> rejected ---
colorUnpack(4294967296):    bool(false)
colorUnpack(PHP_INT_MAX):   bool(false)
addFormatFromStyle(2**32):  bool(false)
packDateValues(huge year):  bool(false)
--- ConditionalFormat / Table setters -> rejected ---
CF setBorder(PHP_INT_MAX):  bool(false)
CF setBorderColor(-1):      bool(false)
CF setNumFormat(PHP_INT_MAX):bool(false)
CF setFillPattern(PHP_INT_MAX):bool(false)
Table setStyle(PHP_INT_MAX):bool(false)
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
