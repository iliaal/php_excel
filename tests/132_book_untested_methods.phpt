--TEST--
ExcelBook::addPictureAsLink, ExcelBook::conditionalFormat, ExcelSheet::removeComment
--SKIPIF--
<?php if (!extension_loaded("excel") || !method_exists("ExcelBook", "conditionalFormatSize")) print "skip"; ?>
--FILE--
<?php

// --- ExcelBook::addPictureAsLink ---

$book = new ExcelBook(null, null, true);

// Create a minimal valid 1x1 PNG file
$tmpPic = tempnam("/tmp", "xlpic") . ".png";
$png = base64_decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==");
file_put_contents($tmpPic, $png);

// Without optional $insert parameter (defaults to false)
$id1 = $book->addPictureAsLink($tmpPic);
var_dump(is_int($id1) && $id1 >= 0);

// With $insert = true
$id2 = $book->addPictureAsLink($tmpPic, true);
var_dump(is_int($id2) && $id2 >= 0);

// With $insert = false
$id3 = $book->addPictureAsLink($tmpPic, false);
var_dump(is_int($id3) && $id3 >= 0);

// Non-existent file returns false
var_dump(@$book->addPictureAsLink("/tmp/nonexistent_xlpic_test.png"));

unlink($tmpPic);

// --- ExcelBook::conditionalFormat ---

$book2 = new ExcelBook(null, null, true);

$cf1 = $book2->addConditionalFormat();
var_dump($cf1 instanceof ExcelConditionalFormat);

$cf2 = $book2->addConditionalFormat();
var_dump($cf2 instanceof ExcelConditionalFormat);

echo "conditionalFormatSize: " . $book2->conditionalFormatSize() . "\n";

// Retrieve by index
$retrieved0 = $book2->conditionalFormat(0);
var_dump($retrieved0 instanceof ExcelConditionalFormat);

$retrieved1 = $book2->conditionalFormat(1);
var_dump($retrieved1 instanceof ExcelConditionalFormat);

// Invalid index returns false
var_dump($book2->conditionalFormat(999));

// --- ExcelSheet::removeComment (XLS format -- comments not readable in xlsx without save/load) ---

$book3 = new ExcelBook();
$sheet3 = $book3->addSheet("Comments");

// Write a comment
$sheet3->writeComment(1, 0, "Test comment", "Author", 100, 50);

// Verify comment exists
$comment = $sheet3->readComment(1, 0);
echo "comment: " . $comment . "\n";

// Remove the comment
$sheet3->removeComment(1, 0);

// Verify comment is gone
var_dump($sheet3->readComment(1, 0));

// Removing a non-existent comment should not error
$sheet3->removeComment(2, 0);

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
conditionalFormatSize: 2
bool(true)
bool(true)
bool(false)
comment: Test comment
bool(false)
OK
