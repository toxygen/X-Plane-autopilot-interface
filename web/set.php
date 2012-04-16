<?php
$infile = "data.in";
$fh = fopen($infile, 'w') or die("can't open infile");
$speed = $_POST['speed'];

fwrite($fh, $speed . PHP_EOL);
fclose($fh);
?>
