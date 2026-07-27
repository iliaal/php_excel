--TEST--
ExcelBook::save() still works with a stream wrapper that omits rename()
--EXTENSIONS--
excel
--FILE--
<?php
/* Object-store wrappers (S3, GCS, Flysystem adapters) commonly implement only
 * the read/write methods. PHP installs rename/unlink dispatchers in the wrapper
 * ops for every user wrapper regardless, so the atomic-save capability probe
 * cannot rule them out up front; save() has to recover after the rename fails. */
class NoRenameStream
{
    public $context;
    private $handle;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        $this->handle = fopen(substr($path, strlen('no-rename://')), $mode);
        return $this->handle !== false;
    }

    public function stream_write(string $data): int { return fwrite($this->handle, $data); }
    public function stream_flush(): bool { return fflush($this->handle); }
    public function stream_close(): void { fclose($this->handle); }
    public function url_stat(string $path, int $flags) { return @stat(substr($path, strlen('no-rename://'))) ?: false; }
}

stream_wrapper_register('no-rename', NoRenameStream::class);
$destination = sys_get_temp_dir() . '/no-rename-destination-' . getmypid() . '.xlsx';

$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'payload');
var_dump(@$book->save('no-rename://' . $destination));

$reloaded = new ExcelBook(null, null, true);
var_dump($reloaded->loadFile($destination));
var_dump($reloaded->getSheet(0)->read(1, 0));

/* Known limitation, unchanged since 2.4.0: a wrapper that implements neither
 * rename() nor unlink() cannot clean up the staged file either, so one is left
 * behind. The destination is still correct, which is what callers depend on. */
var_dump(count(glob($destination . '.*.tmp')) === 1);

foreach (glob($destination . '.*.tmp') as $temporary) {
    @unlink($temporary);
}
@unlink($destination);
echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
string(7) "payload"
bool(true)
OK
