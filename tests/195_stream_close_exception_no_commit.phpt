--TEST--
Stream-backed load does not commit workbook state after a close exception
--EXTENSIONS--
excel
--FILE--
<?php
class ThrowingCloseStream
{
    public $context;
    public static string $data = '';
    private int $position = 0;
    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool { return true; }
    public function stream_read(int $count): string
    {
        $chunk = substr(self::$data, $this->position, $count);
        $this->position += strlen($chunk);
        return $chunk;
    }
    public function stream_eof(): bool { return $this->position >= strlen(self::$data); }
    public function stream_close(): void { throw new RuntimeException('close failed'); }
}

stream_wrapper_register('throwing-close', ThrowingCloseStream::class);
$source = new ExcelBook(null, null, true);
$source->addSheet('Loaded')->write(1, 0, 'payload');
ThrowingCloseStream::$data = $source->save();
$target = new ExcelBook(null, null, true);
$target->addSheet('Before');
try {
    @$target->loadFile('throwing-close://book');
    echo "missing exception\n";
} catch (Throwable $e) {
    echo $e->getMessage(), "\n";
}
var_dump($target->getSheetName(0));
?>
--EXPECT--
close failed
string(6) "Before"
