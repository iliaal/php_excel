--TEST--
ExcelFont::size() rejects a non-positive size instead of silently acting as a getter
--EXTENSIONS--
excel
--DESCRIPTION--
CR-021: size(0) / size(negative) passed the setter guard's `size > 0` test and
fell through to the getter, silently returning the current size with no signal
that the set was ignored. An explicit non-positive size is now rejected
(warning + false), while getter mode (no argument / null) and valid sizes work.
--FILE--
<?php
$b = new ExcelBook(null, null, true);
$f = $b->addFont();

echo "getter (no arg):   "; var_dump($f->size());
echo "set 14:            "; var_dump($f->size(14));
echo "getter after set:  "; var_dump($f->size());
echo "getter (null):     "; var_dump($f->size(null));
echo "size(0):           "; var_dump(@$f->size(0));
echo "unchanged after 0: "; var_dump($f->size());
echo "size(-5):          "; var_dump(@$f->size(-5));
echo "unchanged after -5:"; var_dump($f->size());
echo "OK\n";
?>
--EXPECT--
getter (no arg):   int(11)
set 14:            int(14)
getter after set:  int(14)
getter (null):     int(14)
size(0):           bool(false)
unchanged after 0: int(14)
size(-5):          bool(false)
unchanged after -5:int(14)
OK
