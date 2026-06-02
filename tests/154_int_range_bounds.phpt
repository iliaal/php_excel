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

echo "--- valid values accepted ---\n";
echo "setZoom(120):               "; var_dump($s->setZoom(120));
echo "setPrintFit(1,1):           "; var_dump($s->setPrintFit(1, 1));
echo "setTabRgbColor(10,20,30):   "; var_dump($s->setTabRgbColor(10, 20, 30));
echo "setBorder(1,2,0,1,1,0):     "; var_dump($s->setBorder(1, 2, 0, 1, 1, 0));

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
--- valid values accepted ---
setZoom(120):               NULL
setPrintFit(1,1):           bool(true)
setTabRgbColor(10,20,30):   bool(true)
setBorder(1,2,0,1,1,0):     bool(true)
OK
