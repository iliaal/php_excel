--TEST--
Stream save falls back to a direct write when a staged rename fails
--EXTENSIONS--
excel
--FILE--
<?php
/* A user-defined wrapper always presents rename/unlink entries in its wrapper
 * ops, whether or not the PHP class implements those methods, so save() cannot
 * tell "wrapper has no rename()" from "rename() returned false". It treats both
 * as a fallback to the direct write: a wrapper that omits rename() keeps
 * working, at the cost of no longer preserving the destination when a rename
 * genuinely fails. */
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

$warnings = [];
set_error_handler(function ($no, $message) use (&$warnings) {
    $warnings[] = $message;
    return true;
});

$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'x');
var_dump($book->save('rename-failure://' . $destination));
restore_error_handler();

/* The caller is told the save was not atomic. */
var_dump(count(array_filter($warnings, fn($m) => str_contains($m, 'non-atomic direct write'))) === 1);

/* The workbook landed, and the staged copy did not leak. */
$reloaded = new ExcelBook(null, null, true);
var_dump($reloaded->loadFile($destination));
var_dump($reloaded->getSheet(0)->read(1, 0));
var_dump(count(glob($destination . '.*.tmp')));

foreach (glob($destination . '.*.tmp') as $temporary) {
    @unlink($temporary);
}
@unlink($destination);
echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
string(1) "x"
int(0)
OK
