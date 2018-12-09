<?php

$init = 1000;
$start_time = microtime(true);

while (true) {
	if (strlen($init) % 2 != 0) {
		$init = $init * 10;
	}

	$splitter = strlen($init) / 2;
	$fore = substr($init, 0, $splitter);
	$post = substr($init, $splitter, strlen($init) - 1);

	if (($fore + $post)**2 == $init) {
		printf("%d %d %d %f\n", $fore, $post, $init, microtime(true) - $start_time);
	}

	$init = $init + 1;
}

?>
