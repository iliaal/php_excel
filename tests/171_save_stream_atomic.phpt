--TEST--
Stream-path save() does not destroy the destination on a short write
--EXTENSIONS--
excel
--DESCRIPTION--
CR-001: the stream-path save() opened the destination with "wb" (truncating)
and then wrote; a short write (disk full / interrupted) left the caller's file
destroyed. save() now stages the buffer to a sibling temp URL and renames it
into place, so a failed write leaves the existing file untouched.
--FILE--
<?php
// Stream wrapper that backs a real file, truncates on open, and short-writes
// (accepts only the first 8 bytes) to simulate a disk-full failure.
class FailWrite {
    public $context;
    private $fh;
    private $written = 0;
    function stream_open($path, $mode, $options, &$opened_path) {
        $this->fh = fopen(substr($path, strlen("failwrite://")), "wb");
        return $this->fh !== false;
    }
    function stream_write($data) {
        if ($this->written >= 8) return 0;
        $chunk = substr($data, 0, 8);
        fwrite($this->fh, $chunk);
        $this->written += strlen($chunk);
        return strlen($chunk);
    }
    function stream_close() { if ($this->fh) fclose($this->fh); }
    function stream_flush() { return true; }
}
stream_wrapper_register("failwrite", "FailWrite");

$dest = sys_get_temp_dir() . "/171_cr001_dest.xlsx";
$payload = "ORIGINAL_PAYLOAD_MUST_SURVIVE";
file_put_contents($dest, $payload);

$b = new ExcelBook(null, null, true);
$s = $b->addSheet("S");
$s->write(1, 0, "hello");
$ret = @$b->save("failwrite://" . $dest);

echo "save ret:            "; var_dump($ret);
echo "destination intact:  "; var_dump(file_get_contents($dest) === $payload);

// Clean up any leftover temp sibling and the destination.
foreach (glob($dest . ".*.tmp") as $t) @unlink($t);
@unlink($dest);
echo "OK\n";
?>
--EXPECT--
save ret:            bool(false)
destination intact:  bool(true)
OK
