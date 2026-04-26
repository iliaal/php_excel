# Security policy

php_excel is a PHP extension wrapping the commercial LibXL C library.
Most realistic threat surface is parsing untrusted XLS/XLSX files.

## Supported versions

| Version | Supported          |
|---------|--------------------|
| 2.0.x   | :white_check_mark: |

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
