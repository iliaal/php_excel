--TEST--
FormControl: read properties, setters, item manipulation on all control types
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$book->loadFile(__DIR__ . '/formcontrols.xlsx');
$sheet = $book->getSheet(0);
$count = $sheet->formControlSize();
echo "count: $count\n";

// === Checkbox (index 0, type=OBJECT_CHECKBOX=2) ===
$cb = $sheet->formControl(0);
echo "cb type: " . $cb->objectType() . "\n";
echo "cb name: " . $cb->name() . "\n";
echo "cb checked: " . $cb->checked() . "\n";
echo "cb fmlaLink: " . $cb->fmlaLink() . "\n";
echo "cb locked: " . var_export($cb->locked(), true) . "\n";
echo "cb defaultSize: " . var_export($cb->defaultSize(), true) . "\n";
echo "cb print: " . var_export($cb->print(), true) . "\n";
echo "cb disabled: " . var_export($cb->disabled(), true) . "\n";
$from = $cb->fromAnchor();
$to = $cb->toAnchor();
echo "cb fromAnchor col: " . $from['col'] . " row: " . $from['row'] . "\n";
echo "cb toAnchor col: " . $to['col'] . " row: " . $to['row'] . "\n";

// Checkbox setters
$cb->setFmlaLink('$G$1');
echo "cb fmlaLink after set: " . $cb->fmlaLink() . "\n";
$cb->setFmlaGroup('$H$1:$H$5');
echo "cb fmlaGroup: " . $cb->fmlaGroup() . "\n";
$cb->setFmlaTxbx('$I$1');
echo "cb fmlaTxbx: " . $cb->fmlaTxbx() . "\n";
$cb->setFmlaRange('$J$1:$J$3');
echo "cb fmlaRange: " . $cb->fmlaRange() . "\n";

// === Dropdown (index 1, type=OBJECT_DROP=3) ===
$dd = $sheet->formControl(1);
echo "dd type: " . $dd->objectType() . "\n";
echo "dd name: " . $dd->name() . "\n";
echo "dd fmlaLink: " . $dd->fmlaLink() . "\n";
echo "dd fmlaRange: " . $dd->fmlaRange() . "\n";
echo "dd dropLines: " . $dd->dropLines() . "\n";
echo "dd dx: " . $dd->dx() . "\n";

$dd->setDropLines(12);
echo "dd dropLines after set: " . $dd->dropLines() . "\n";
$dd->setSel(2);
echo "dd sel after set: " . $dd->sel() . "\n";
$dd->setDx(30);
echo "dd dx after set: " . $dd->dx() . "\n";

// === Spinner (index 2, type=OBJECT_SPIN=9) ===
$sp = $sheet->formControl(2);
echo "sp type: " . $sp->objectType() . "\n";
echo "sp name: " . $sp->name() . "\n";
echo "sp min: " . $sp->getMin() . "\n";
echo "sp max: " . $sp->getMax() . "\n";
echo "sp inc: " . $sp->inc() . "\n";
echo "sp fmlaLink: " . $sp->fmlaLink() . "\n";
echo "sp horiz: " . var_export($sp->horiz(), true) . "\n";

$sp->setMin(10);
echo "sp min after set: " . $sp->getMin() . "\n";
$sp->setMax(500);
echo "sp max after set: " . $sp->getMax() . "\n";
$sp->setInc(25);
echo "sp inc after set: " . $sp->inc() . "\n";
$sp->setHoriz(true);
echo "sp horiz after set: " . var_export($sp->horiz(), true) . "\n";
$sp->setFirstButton(true);
echo "sp firstButton after set: " . var_export($sp->firstButton(), true) . "\n";

// === Listbox (index 3, type=OBJECT_LIST=6) ===
$lb = $sheet->formControl(3);
echo "lb type: " . $lb->objectType() . "\n";
echo "lb name: " . $lb->name() . "\n";
echo "lb fmlaLink: " . $lb->fmlaLink() . "\n";
echo "lb fmlaRange: " . $lb->fmlaRange() . "\n";
echo "lb sel: " . $lb->sel() . "\n";

// Item manipulation
echo "lb itemSize: " . $lb->itemSize() . "\n";
$lb->addItem("Durian");
$lb->addItem("Elderberry");
echo "lb itemSize after adds: " . $lb->itemSize() . "\n";
echo "lb item(0): " . $lb->item(0) . "\n";
echo "lb item(1): " . $lb->item(1) . "\n";
$lb->insertItem(0, "Avocado");
echo "lb itemSize after insert: " . $lb->itemSize() . "\n";
echo "lb item(0): " . $lb->item(0) . "\n";
echo "lb item(1): " . $lb->item(1) . "\n";
echo "lb item(2): " . $lb->item(2) . "\n";
$lb->clearItems();
echo "lb itemSize after clear: " . $lb->itemSize() . "\n";

$lb->setMultiSel("1 0 1");
echo "lb multiSel: " . $lb->multiSel() . "\n";

// === Button (index 4, type=OBJECT_BUTTON=1) ===
$btn = $sheet->formControl(4);
echo "btn type: " . $btn->objectType() . "\n";
echo "btn name: " . $btn->name() . "\n";
echo "btn locked: " . var_export($btn->locked(), true) . "\n";

// === Scrollbar (index 5, type=OBJECT_SCROLL=8) ===
$sb = $sheet->formControl(5);
echo "sb type: " . $sb->objectType() . "\n";
echo "sb name: " . $sb->name() . "\n";
echo "sb min: " . $sb->getMin() . "\n";
echo "sb max: " . $sb->getMax() . "\n";
echo "sb inc: " . $sb->inc() . "\n";
echo "sb horiz: " . var_export($sb->horiz(), true) . "\n";
echo "sb fmlaLink: " . $sb->fmlaLink() . "\n";

// === Group box (index 6, type=OBJECT_GBOX=4) ===
$gb = $sheet->formControl(6);
echo "gb type: " . $gb->objectType() . "\n";
echo "gb name: " . $gb->name() . "\n";

// === Edge cases ===
var_dump($sheet->formControl($count));
var_dump(@$sheet->formControl(-1));

// Out-of-int-range setter args are rejected before reaching libxl.
echo "sb setMin(PHP_INT_MAX): " . var_export(@$sb->setMin(PHP_INT_MAX), true) . "\n";
echo "sb setMax(-1): " . var_export(@$sb->setMax(-1), true) . "\n";
echo "sb setInc(PHP_INT_MAX): " . var_export(@$sb->setInc(PHP_INT_MAX), true) . "\n";
echo "sb setMin(0) valid: " . var_export($sb->setMin(0), true) . "\n";

echo "OK\n";
?>
--EXPECT--
count: 7
cb type: 2
cb name: Check Box 1
cb checked: 1
cb fmlaLink: $B$1
cb locked: true
cb defaultSize: false
cb print: true
cb disabled: false
cb fromAnchor col: 0 row: 1
cb toAnchor col: 3 row: 2
cb fmlaLink after set: $G$1
cb fmlaGroup: $H$1:$H$5
cb fmlaTxbx: $I$1
cb fmlaRange: $J$1:$J$3
dd type: 3
dd name: Drop Down 2
dd fmlaLink: $C$1
dd fmlaRange: $A$10:$C$10
dd dropLines: 8
dd dx: 26
dd dropLines after set: 12
dd sel after set: 2
dd dx after set: 30
sp type: 9
sp name: Spinner 3
sp min: 0
sp max: 100
sp inc: 5
sp fmlaLink: $D$1
sp horiz: false
sp min after set: 10
sp max after set: 500
sp inc after set: 25
sp horiz after set: true
sp firstButton after set: true
lb type: 6
lb name: List Box 4
lb fmlaLink: $E$1
lb fmlaRange: $A$10:$C$10
lb sel: 1
lb itemSize: 0
lb itemSize after adds: 2
lb item(0): Durian
lb item(1): Elderberry
lb itemSize after insert: 3
lb item(0): Avocado
lb item(1): Durian
lb item(2): Elderberry
lb itemSize after clear: 0
lb multiSel: 1 0 1
btn type: 1
btn name: Button 5
btn locked: true
sb type: 8
sb name: Scroll Bar 6
sb min: 0
sb max: 200
sb inc: 1
sb horiz: true
sb fmlaLink: $F$1
gb type: 4
gb name: Group Box 7
bool(false)
bool(false)
sb setMin(PHP_INT_MAX): false
sb setMax(-1): false
sb setInc(PHP_INT_MAX): false
sb setMin(0) valid: true
OK
