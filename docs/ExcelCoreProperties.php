<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelCoreProperties                                                       |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelCoreProperties
{
	/**
	* Create a core properties object for a workbook
	*
	* @see ExcelBook::coreProperties()
	* @param ExcelBook $book
	*/
	public function __construct(ExcelBook $book)
	{
	}

	/**
	* Returns the title
	*
	* @return string|null|false
	*/
	public function title(): string|null|false
	{
	}

	/**
	* Sets the title
	*
	* @param string $value
	* @return bool
	*/
	public function setTitle($value): bool
	{
	}

	/**
	* Returns the subject
	*
	* @return string|null|false
	*/
	public function subject(): string|null|false
	{
	}

	/**
	* Sets the subject
	*
	* @param string $value
	* @return bool
	*/
	public function setSubject($value): bool
	{
	}

	/**
	* Returns the creator
	*
	* @return string|null|false
	*/
	public function creator(): string|null|false
	{
	}

	/**
	* Sets the creator
	*
	* @param string $value
	* @return bool
	*/
	public function setCreator($value): bool
	{
	}

	/**
	* Returns the last modified by value
	*
	* @return string|null|false
	*/
	public function lastModifiedBy(): string|null|false
	{
	}

	/**
	* Sets the last modified by value
	*
	* @param string $value
	* @return bool
	*/
	public function setLastModifiedBy($value): bool
	{
	}

	/**
	* Returns the created date as a string
	*
	* @return string|null|false
	*/
	public function created(): string|null|false
	{
	}

	/**
	* Sets the created date as a string
	*
	* @param string $value
	* @return bool
	*/
	public function setCreated($value): bool
	{
	}

	/**
	* Returns the modified date as a string
	*
	* @return string|null|false
	*/
	public function modified(): string|null|false
	{
	}

	/**
	* Sets the modified date as a string
	*
	* @param string $value
	* @return bool
	*/
	public function setModified($value): bool
	{
	}

	/**
	* Returns the tags
	*
	* @return string|null|false
	*/
	public function tags(): string|null|false
	{
	}

	/**
	* Sets the tags
	*
	* @param string $value
	* @return bool
	*/
	public function setTags($value): bool
	{
	}

	/**
	* Returns the categories
	*
	* @return string|null|false
	*/
	public function categories(): string|null|false
	{
	}

	/**
	* Sets the categories
	*
	* @param string $value
	* @return bool
	*/
	public function setCategories($value): bool
	{
	}

	/**
	* Returns the comments
	*
	* @return string|null|false
	*/
	public function comments(): string|null|false
	{
	}

	/**
	* Sets the comments
	*
	* @param string $value
	* @return bool
	*/
	public function setComments($value): bool
	{
	}

	/**
	* Returns the created date as a double (Excel timestamp)
	*
	* @return float
	*/
	public function createdAsDouble()
	{
	}

	/**
	* Sets the created date as a double (Excel timestamp)
	*
	* @param float $value
	* @return bool
	*/
	public function setCreatedAsDouble($value): bool
	{
	}

	/**
	* Returns the modified date as a double (Excel timestamp)
	*
	* @return float
	*/
	public function modifiedAsDouble()
	{
	}

	/**
	* Sets the modified date as a double (Excel timestamp)
	*
	* @param float $value
	* @return bool
	*/
	public function setModifiedAsDouble($value): bool
	{
	}

	/**
	* Removes all core properties
	*
	* @return void
	*/
	public function removeAll()
	{
	}

} // end ExcelCoreProperties
