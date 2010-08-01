--TEST--
Font styling tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) print "skip"; ?>
--FILE--
<?php 
	$x = new ExcelBook();

	$s = $x->addSheet("Sheet 1");

	for ($i = 0; $i < 40; $i++) {
		$font = $x->addFont();
		$font->size($i);
		$format = $x->addFormat();
		$format->setFont($font);

		var_dump($s->write(1, $i, 'test', $format));
		var_dump($x->getError());				

		$fmt = '';
		$s->read(1, $i, $format);
		echo $format->getFont()->size() . "\n";
	}

	$i = 1;
	foreach (array('italics', 'bold', 'strike') as $style) {
		$font = $x->addFont();
		$font->$style(true);
		$format = $x->addFormat();
		$format->setFont($font);

		var_dump($s->write(2, $i, 'test', $format));
		var_dump($x->getError());				

		$fmt = '';
		$s->read(2, $i, $format);
		echo $format->getFont()->$style() . "\n";

		++$i;
	}

	$i = 1;
	$oClass = new ReflectionClass('ExcelFormat');
	foreach ($oClass->getConstants() as $c => $val) {
		if (strpos($c, 'COLOR_') !== 0) {
			continue;
		}

		$font = $x->addFont();
		$font->color($val);
		$format = $x->addFormat();
		$format->setFont($font);

		var_dump($s->write(2, $i, 'test', $format));
		var_dump($x->getError());				

		$fmt = '';
		$s->read(2, $i, $format);
		echo $format->getFont()->color() . "\n";

		++$i;
	}

	$i = 1;
	$oClass = new ReflectionClass('ExcelFont');
	foreach ($oClass->getConstants() as $c => $val) {
		if (strpos($c, 'SCRIPT_') !== 0) {
			continue;
		}

		$font = $x->addFont();
		$font->script($val);
		$format = $x->addFormat();
		$format->setFont($font);

		var_dump($s->write(3, $i, 'test', $format));
		var_dump($x->getError());				

		$fmt = '';
		$s->read(3, $i, $format);
		echo $format->getFont()->script() . "\n";

		++$i;
	}

	$i = 1;
	$oClass = new ReflectionClass('ExcelFont');
	foreach ($oClass->getConstants() as $c => $val) {
		if (strpos($c, 'UNDERLINE_') !== 0) {
			continue;
		}

		$font = $x->addFont();
		$font->underline($val);
		$format = $x->addFormat();
		$format->setFont($font);

		var_dump($s->write(4, $i, 'test', $format));
		var_dump($x->getError());				

		$fmt = '';
		$s->read(4, $i, $format);
		echo $format->getFont()->underline() . "\n";

		++$i;
	}

	$i = 1;
	foreach (array('Arial', 'Helvetica', 'Courier', 'Times New Roman', 'Tahoma', 'Courier New') as $style) {
		$font = $x->addFont();
		$font->name($style);
		$format = $x->addFormat();
		$format->setFont($font);

		var_dump($s->write(5, $i, 'test', $format));
		var_dump($x->getError());				

		$fmt = '';
		$s->read(5, $i, $format);
		echo $format->getFont()->name() . "\n";

		++$i;
	}



	echo "OK\n";
?>
--EXPECT--
bool(true)
bool(false)
10
bool(true)
bool(false)
1
bool(true)
bool(false)
2
bool(true)
bool(false)
3
bool(true)
bool(false)
4
bool(true)
bool(false)
5
bool(true)
bool(false)
6
bool(true)
bool(false)
7
bool(true)
bool(false)
8
bool(true)
bool(false)
9
bool(true)
bool(false)
10
bool(true)
bool(false)
11
bool(true)
bool(false)
12
bool(true)
bool(false)
13
bool(true)
bool(false)
14
bool(true)
bool(false)
15
bool(true)
bool(false)
16
bool(true)
bool(false)
17
bool(true)
bool(false)
18
bool(true)
bool(false)
19
bool(true)
bool(false)
20
bool(true)
bool(false)
21
bool(true)
bool(false)
22
bool(true)
bool(false)
23
bool(true)
bool(false)
24
bool(true)
bool(false)
25
bool(true)
bool(false)
26
bool(true)
bool(false)
27
bool(true)
bool(false)
28
bool(true)
bool(false)
29
bool(true)
bool(false)
30
bool(true)
bool(false)
31
bool(true)
bool(false)
32
bool(true)
bool(false)
33
bool(true)
bool(false)
34
bool(true)
bool(false)
35
bool(true)
bool(false)
36
bool(true)
bool(false)
37
bool(true)
bool(false)
38
bool(true)
bool(false)
39
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
1
bool(true)
bool(false)
8
bool(true)
bool(false)
9
bool(true)
bool(false)
10
bool(true)
bool(false)
11
bool(true)
bool(false)
12
bool(true)
bool(false)
13
bool(true)
bool(false)
14
bool(true)
bool(false)
15
bool(true)
bool(false)
16
bool(true)
bool(false)
17
bool(true)
bool(false)
18
bool(true)
bool(false)
19
bool(true)
bool(false)
20
bool(true)
bool(false)
21
bool(true)
bool(false)
22
bool(true)
bool(false)
23
bool(true)
bool(false)
24
bool(true)
bool(false)
25
bool(true)
bool(false)
26
bool(true)
bool(false)
27
bool(true)
bool(false)
28
bool(true)
bool(false)
29
bool(true)
bool(false)
30
bool(true)
bool(false)
31
bool(true)
bool(false)
32
bool(true)
bool(false)
33
bool(true)
bool(false)
34
bool(true)
bool(false)
35
bool(true)
bool(false)
36
bool(true)
bool(false)
37
bool(true)
bool(false)
38
bool(true)
bool(false)
39
bool(true)
bool(false)
40
bool(true)
bool(false)
41
bool(true)
bool(false)
42
bool(true)
bool(false)
43
bool(true)
bool(false)
44
bool(true)
bool(false)
45
bool(true)
bool(false)
46
bool(true)
bool(false)
47
bool(true)
bool(false)
48
bool(true)
bool(false)
49
bool(true)
bool(false)
50
bool(true)
bool(false)
51
bool(true)
bool(false)
52
bool(true)
bool(false)
53
bool(true)
bool(false)
54
bool(true)
bool(false)
55
bool(true)
bool(false)
56
bool(true)
bool(false)
57
bool(true)
bool(false)
58
bool(true)
bool(false)
59
bool(true)
bool(false)
60
bool(true)
bool(false)
61
bool(true)
bool(false)
62
bool(true)
bool(false)
63
bool(true)
bool(false)
64
bool(true)
bool(false)
65
bool(true)
bool(false)
0
bool(true)
bool(false)
1
bool(true)
bool(false)
2
bool(true)
bool(false)
33
bool(true)
bool(false)
34
bool(true)
bool(false)
Arial
bool(true)
bool(false)
Helvetica
bool(true)
bool(false)
Courier
bool(true)
bool(false)
Times New Roman
bool(true)
bool(false)
Tahoma
bool(true)
bool(false)
Courier New
OK
