--TEST--
Stream save fails without touching the destination when a staged rename fails
--EXTENSIONS--
excel
--FILE--
<?php
/* This wrapper implements rename() and fails it, so the save fails closed:
 * it returns false with a "destination left unchanged" warning, the
 * destination keeps its original content, and the staged copy is removed
 * instead of falling back to a non-atomic direct write. Wrappers whose class
 * omits rename() still take the non-atomic direct-write fallback (see 215). */
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

/* The caller is told the destination was left unchanged. */
var_dump(count(array_filter($warnings, fn($m) => str_contains($m, 'destination left unchanged'))) === 1);

/* The destination keeps its original content, and the staged copy did not leak. */
var_dump(file_get_contents($destination) === 'ORIGINAL');
var_dump(count(glob($destination . '.*.tmp')));

foreach (glob($destination . '.*.tmp') as $temporary) {
    @unlink($temporary);
}
@unlink($destination);
echo "OK\n";
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
int(0)
OK
