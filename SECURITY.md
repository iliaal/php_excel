# Security policy

php_excel is a PHP extension wrapping the commercial LibXL C library.
Most realistic threat surface is parsing untrusted XLS/XLSX files.

## Supported versions

| Version | Supported          |
|---------|--------------------|
| 2.x     | :white_check_mark: |

Once 3.0 ships, the two most recent minor versions will receive
security fixes. Pre-2.0 (PHP 5/7 era) is unsupported.

## Reporting a vulnerability

**Do not file a public GitHub issue for security vulnerabilities.**

Use GitHub's private security advisory feature at
<https://github.com/iliaal/php_excel/security/advisories/new>
or email Ilia Alshanetsky <ilia@ilia.ws> directly.

Please include:

- Affected php_excel version (`php -r 'echo phpversion("excel");'`)
- Affected LibXL version (`ExcelBook::getLibXlVersion()`)
- A minimal reproducing case (PHP code + the .xls/.xlsx fixture that
  triggers it; small enough to inline in the report)
- Impact: crash / RCE / info disclosure / DoS / etc.
- Whether you've coordinated disclosure with anyone else

Acknowledgement within 7 days, fix or status update within 30. Once a
fix is released the advisory becomes public.

## Writing untrusted values (formula injection)

When no explicit data type is passed, `ExcelSheet::write()`, `writeRow()`, and
`writeCol()` promote a string whose first character is `=` to a live formula.
Writing untrusted user input this way is a spreadsheet formula-injection
vector: a value like `=HYPERLINK(...)` or `=cmd|...` can execute or exfiltrate
when the file is opened in Excel.

When writing values you do not control, pass `ExcelFormat::AS_TEXT` as the data
type so the string is stored verbatim with no formula promotion, no
leading-quote stripping, and no numeric coercion:

```php
$sheet->write($row, $col, (string) $untrusted, null, ExcelFormat::AS_TEXT);
$sheet->writeRow($row, array_map('strval', $untrusted_array), 0, null, ExcelFormat::AS_TEXT);
```

`AS_TEXT` describes a string cell, so it accepts strings (and `null`, which
writes a blank). Decoded JSON and form data routinely carry ints, floats and
booleans, and passing one of those with an explicit data type is rejected
rather than silently stored as another kind. Cast at the call site as above;
for `writeRow()` and `writeCol()` a single unconvertible element rejects the
whole row or column, leaving no cells modified.

Implicit `=` promotion is retained as the default for backwards compatibility;
it may be revisited in a future major version.

## External links (picture links, hyperlinks)

`ExcelBook::addPictureAsLink()` stores the caller-supplied path as an
external relationship: UNC paths and http(s) URLs persist in the file and
are resolved by Excel on open, which can leak NTLM hashes (UNC) or signal
file opens (remote URLs). Allowlist caller-supplied paths and never pass
untrusted input as the link target. The same applies to
`ExcelSheet::addHyperlink()`: the string is stored verbatim, so validate
the scheme (e.g. allow only `https:`/`mailto:`) and never store untrusted
strings.

## Memory use when loading (buffering)

`load()`, `loadFile()`, `loadPartially()`, `loadInfo()`, and
`addPictureFromFile()` fully buffer stream sources in memory (capped at
UINT_MAX, ~4 GiB, beyond which the call fails). Enforce an app-side size
bound before calling so one upload cannot exhaust PHP memory:

```php
$maxBytes = 64 * 1024 * 1024; // app policy
$size = filesize($path);
if ($size === false || $size > $maxBytes) {
    throw new RuntimeException('spreadsheet too large');
}
$book->loadFile($path);
```

## Scope

In scope:

- Crashes, memory corruption, or read-after-free in the wrapper code
  (`excel.c`) when parsing crafted XLS/XLSX input via `ExcelBook::load()`,
  `loadFile()`, `loadInfo()`, or `loadInfoRaw()`.
- Buffer overflows or integer overflows in cell-read code paths
  (`read()`, `readRow()`, `readCol()`, `cellFormat()`, etc.) when the
  underlying file declares unusual cell counts, formats, or types.
- Arginfo / ZPP mismatches that cause undefined behavior reachable
  from PHP.
- License-key validation bypasses (`ExcelBook::requiresKey()` /
  `excel.license_name` / `excel.license_key` INI settings).

Out of scope:

- Vulnerabilities in LibXL itself: LibXL is closed-source commercial
  software; report directly to libxl.com. We track LibXL releases and
  bump the minimum supported version when a relevant fix ships
  (currently LibXL 4.6.0 minimum).
- Third-party applications that pass arbitrary user-uploaded files
  to `loadFile()` without size or content-type limits. Treat
  user-uploaded spreadsheets as you would any user-uploaded archive.
- Behavior with `excel.skip_empty` set to non-default values: this
  knob is a backwards-compat aid, not a security boundary.
