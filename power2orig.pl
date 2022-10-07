#!/usr/bin/env perl

use warnings;
use strict;

my $init = 1000;
my $init_time = time();

while (1) {
	if (length($init) % 2 != 0) {
		$init = $init * 10;
	}
	my $splitter = length($init) / 2;

	my $fore = substr $init, 0, $splitter;
	my $post = substr $init, $splitter, $splitter;

	if (($fore + $post) ** 2 == $init) {
		my $found_time = time() - $init_time;
		print "$fore $post $init $found_time\n";
	}
	$init++;
}
