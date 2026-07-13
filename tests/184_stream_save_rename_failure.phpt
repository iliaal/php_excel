--TEST--
Stream save preserves the destination when a staged rename fails
--EXTENSIONS--
excel
--FILE--
<?php
class RenameFailureStream
{
    public $context;
    private $handle;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        $this->handle = fopen(substr($path, strlen('rename-failure://')), 'wb');
        return $this->handle !== false;
    }

    public function stream_write(string $data): int { return fwrite($this->handle, $data); }
    public function stream_flush(): bool { return true; }
    public function stream_close(): void { fclose($this->handle); }
    public function unlink(string $path): bool { return @unlink(substr($path, strlen('rename-failure://'))); }
    public function rename(string $from, string $to): bool { return false; }
}

stream_wrapper_register('rename-failure', RenameFailureStream::class);
$destination = sys_get_temp_dir() . '/rename-failure-destination.xlsx';
file_put_contents($destination, 'ORIGINAL');

$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'x');
var_dump(@$book->save('rename-failure://' . $destination));
var_dump(file_get_contents($destination));

foreach (glob($destination . '.*.tmp') as $temporary) {
    @unlink($temporary);
}
@unlink($destination);
echo "OK\n";
?>
--EXPECT--
bool(false)
string(8) "ORIGINAL"
OK
