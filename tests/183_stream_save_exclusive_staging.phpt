--TEST--
Stream save does not follow a pre-created predictable staging symlink
--EXTENSIONS--
excel
--FILE--
<?php
class FilesystemStream
{
    public $context;
    private $handle;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        $this->handle = @fopen(substr($path, strlen('stage-file://')), $mode);
        return $this->handle !== false;
    }

    public function stream_write(string $data): int { return fwrite($this->handle, $data); }
    public function stream_flush(): bool { return fflush($this->handle); }
    public function stream_close(): void { fclose($this->handle); }
    public function unlink(string $path): bool { return @unlink(substr($path, strlen('stage-file://'))); }
    public function rename(string $from, string $to): bool
    {
        return rename(substr($from, strlen('stage-file://')), substr($to, strlen('stage-file://')));
    }
}

stream_wrapper_register('stage-file', FilesystemStream::class);
$destination = sys_get_temp_dir() . '/exclusive-stage-destination.xlsx';
$victim = sys_get_temp_dir() . '/exclusive-stage-victim';
$oldTemp = $destination . '.' . getmypid() . '.tmp';
file_put_contents($victim, 'VICTIM');
@unlink($oldTemp);
symlink($victim, $oldTemp);

$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'x');
var_dump($book->save('stage-file://' . $destination));
var_dump(file_get_contents($victim));
var_dump(str_starts_with(file_get_contents($destination), "PK\x03\x04"));

@unlink($oldTemp);
@unlink($destination);
@unlink($victim);
echo "OK\n";
?>
--EXPECT--
bool(true)
string(6) "VICTIM"
bool(true)
OK
