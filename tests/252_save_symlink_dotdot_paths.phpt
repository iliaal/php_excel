--TEST--
save() through symlinks and ".." writes where the kernel resolves the name, with and without open_basedir
--EXTENSIONS--
excel
--SKIPIF--
<?php if (PHP_OS_FAMILY === "Windows") die("skip symlink() needs elevated privileges on Windows"); ?>
--FILE--
<?php
$root = sys_get_temp_dir() . '/excel-252-' . bin2hex(random_bytes(6));
mkdir($root);

function excel_252_book(string $text): ExcelBook
{
    $book = new ExcelBook(null, null, true);
    $book->addSheet('S')->write(1, 1, $text);
    return $book;
}

function excel_252_read(string $name): string
{
    $book = new ExcelBook(null, null, true);
    return @$book->loadFile($name) ? (string) $book->getSheet(0)->read(1, 1) : 'unloadable';
}

function excel_252_stray(string $dir): int
{
    $count = 0;
    foreach (scandir($dir) as $f) {
        if ($f === '.' || $f === '..') {
            continue;
        }
        if (str_ends_with($f, '.tmp')) {
            $count++;
        }
        if (is_dir("$dir/$f") && !is_link("$dir/$f")) {
            $count += excel_252_stray("$dir/$f");
        }
    }
    return $count;
}

foreach (['plain', 'open_basedir'] as $mode) {
    $d = "$root/$mode";
    mkdir("$d/a/b", 0777, true);
    symlink("$d/a/b", "$d/sl");
    excel_252_book('old')->save("$d/target.xlsx");
    symlink("$d/target.xlsx", "$d/link.xlsx");
    excel_252_book('old')->save("$d/target-abs.xlsx");
    symlink("$d/target-abs.xlsx", "$d/link-abs.xlsx");
    symlink("$d/missing.xlsx", "$d/dangling.xlsx");
    excel_252_book('old')->save("$d/mode.xlsx");
    chmod("$d/mode.xlsx", 0640);

    if ($mode === 'open_basedir') {
        ini_set('open_basedir', $root);
    }
    chdir($d);
    echo "== $mode\n";

    $ok = excel_252_book('new')->save('link.xlsx');
    clearstatcache();
    echo "symlink destination: ", var_export($ok, true), " replaced=", var_export(!is_link("$d/link.xlsx"), true),
        " ", excel_252_read("$d/link.xlsx"), " target=", excel_252_read("$d/target.xlsx"), "\n";

    $ok = excel_252_book('new')->save("$d/link-abs.xlsx");
    clearstatcache();
    echo "absolute symlink destination: ", var_export($ok, true), " replaced=", var_export(!is_link("$d/link-abs.xlsx"), true),
        " ", excel_252_read("$d/link-abs.xlsx"), " target=", excel_252_read("$d/target-abs.xlsx"), "\n";

    $ok = excel_252_book('new')->save('dangling.xlsx');
    clearstatcache();
    echo "dangling symlink destination: ", var_export($ok, true), " replaced=", var_export(!is_link("$d/dangling.xlsx"), true),
        " ", excel_252_read("$d/dangling.xlsx"), " target_created=", var_export(file_exists("$d/missing.xlsx"), true), "\n";

    foreach (['sl/../rel.xlsx' => 'rel.xlsx', "$d/sl/../abs.xlsx" => 'abs.xlsx'] as $name => $base) {
        $ok = excel_252_book('new')->save($name);
        clearstatcache();
        echo ($name[0] === '/' ? 'absolute' : 'relative'), " sl/..: ", var_export($ok, true),
            " in_a=", var_export(is_file("$d/a/$base") && filesize("$d/a/$base") > 0, true),
            " in_top=", var_export(file_exists("$d/$base"), true), " ", excel_252_read($name), "\n";
    }

    $ok = excel_252_book('new')->save('sl/dir.xlsx');
    clearstatcache();
    echo "symlinked directory: ", var_export($ok, true), " in_b=", var_export(is_file("$d/a/b/dir.xlsx"), true),
        " ", excel_252_read('sl/dir.xlsx'), "\n";

    $ok = excel_252_book('new')->save('mode.xlsx');
    clearstatcache();
    echo "mode kept: ", var_export($ok, true), " ", decoct(fileperms("$d/mode.xlsx") & 0777), " ", excel_252_read('mode.xlsx'), "\n";

    echo "stray temporary files: ", excel_252_stray($d), "\n";
    chdir($root);
}
?>
--CLEAN--
<?php
function excel_252_remove(string $path): void
{
    if (is_link($path) || is_file($path)) {
        @unlink($path);
        return;
    }
    if (is_dir($path)) {
        foreach (scandir($path) as $f) {
            if ($f !== '.' && $f !== '..') {
                excel_252_remove("$path/$f");
            }
        }
        @rmdir($path);
    }
}
foreach (glob(sys_get_temp_dir() . '/excel-252-*') as $dir) {
    excel_252_remove($dir);
}
foreach (['link', 'link-abs', 'dangling', 'mode', 'rel', 'abs', 'dir'] as $base) {
    foreach (glob(getcwd() . "/$base.xlsx.*.tmp") as $f) {
        @unlink($f);
    }
}
?>
--EXPECT--
== plain
symlink destination: true replaced=true new target=old
absolute symlink destination: true replaced=true new target=old
dangling symlink destination: true replaced=true new target_created=false
relative sl/..: true in_a=true in_top=false new
absolute sl/..: true in_a=true in_top=false new
symlinked directory: true in_b=true new
mode kept: true 640 new
stray temporary files: 0
== open_basedir
symlink destination: true replaced=true new target=old
absolute symlink destination: true replaced=true new target=old
dangling symlink destination: true replaced=true new target_created=false
relative sl/..: true in_a=true in_top=false new
absolute sl/..: true in_a=true in_top=false new
symlinked directory: true in_b=true new
mode kept: true 640 new
stray temporary files: 0
