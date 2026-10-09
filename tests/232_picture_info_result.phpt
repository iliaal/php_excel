--TEST--
Picture metadata preserves all nine fields and returns independent arrays in XLS and XLSX
--EXTENSIONS--
excel
--FILE--
<?php
$keys = ['picture_index', 'row_top', 'col_left', 'row_bottom', 'col_right',
    'width', 'height', 'offset_x', 'offset_y'];
foreach ([false, true] as $xlsx) {
    echo $xlsx ? "XLSX\n" : "XLS\n";
    $book = new ExcelBook(null, null, $xlsx);
    $sheet = $book->addSheet('Pictures');
    var_dump($sheet->getPictureInfo(0));
    $id = $book->addPictureFromFile(__DIR__ . '/phplogo.jpg');
    $sheet->addPictureDim(1, 1, $id, 57, 40, 3, 4);
    $info = $sheet->getPictureInfo(0);
    var_dump(array_keys($info) === $keys);
    var_dump(count(array_filter($info, 'is_int')) === 9);
    var_dump($info['picture_index'] === $id);
    var_dump($info['row_top'] === 1 && $info['col_left'] === 1);
    var_dump($info['row_bottom'] >= $info['row_top'] && $info['col_right'] >= $info['col_left']);
    var_dump($info['width'] === 57 && $info['height'] === 40);
    var_dump($info['offset_x'] === 3 && $info['offset_y'] === 4);
    $original = $info;
    $info['offset_y'] = 999;
    unset($info['picture_index']);
    var_dump($sheet->getPictureInfo(0) === $original);
    var_dump($sheet->getPictureInfo(1));
}
?>
--EXPECT--
XLS
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
XLSX
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
