# Contributing to php_excel

## Requirements

- PHP 8.3+ (debug build recommended: `--enable-debug`)
- [LibXL](http://www.libxl.com/) 4.6.0+
- Standard C build tools (gcc, make, autoconf)

## Bug reports

Use the [GitHub issue tracker](https://github.com/iliaal/php_excel/issues). Include:

- PHP version (`php -v`)
- LibXL version
- Operating system
- Minimal reproducing code
- Expected vs actual behavior
- Any error messages or crash output

Check if the issue is already reported and try to reproduce with the latest `master` branch before filing.

## Pull requests

1. Fork and clone the repo
2. Create a topic branch off `master`
3. Make your changes
4. Add or update tests in `tests/` (PHPT format)
5. Build and run the test suite:

   ```sh
   phpize
   ./configure --with-excel \
     --with-libxl-incdir=/path/to/libxl/include_c \
     --with-libxl-libdir=/path/to/libxl/lib64
   make

   LD_LIBRARY_PATH=/path/to/libxl/lib64 \
     TEST_PHP_EXECUTABLE=$(which php) \
     TEST_PHP_ARGS="-d extension=$(pwd)/modules/excel.so" \
     NO_INTERACTION=1 php run-tests.php tests/
   ```

6. Verify zero compiler warnings and all tests pass
7. Push and open a PR against `master`

### Test guidelines

- Tests use PHPT format (see existing tests for examples)
- Write to row 1+ (row 0 is blocked by the libxl trial version)
- Keep cell count under ~300 per workbook (trial read limit); create fresh `ExcelBook` instances to reset
- Test both success and failure/edge cases
- Use `new ExcelBook(null, null, true)` for xlsx-specific features

### Code style

Follow the patterns in `excel.c`: tab indentation, `EXCEL_METHOD` macro for method implementations, `ZEND_PARSE_PARAMETERS_NONE()` for zero-arg methods, typed arginfo, `ZEND_THIS` instead of `getThis()`.

By submitting a patch, you agree to license your work under the same license as the project (PHP License 3.01).
