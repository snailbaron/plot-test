#!/usr/bin/env perl

use 5.010;
use strict;
use warnings;

$| = 1;

my $N = 10000000;
my $out_file = 'dots.bin';

open (my $of, '>:raw', $out_file) or die "Unable to open $out_file: $!";
print $of pack('l<', $N);
my $dot_value = 100000;
say "Generating data (. ~ $dot_value points)";
for (my $i = 0; $i < $N; $i++) {
    my $angle = $i / 100;
    print $of pack('f<', $angle);
    print $of pack('f<', sin($angle));
    if ($i % $dot_value == 0) {
        print ".";
    }
}
close $of;
