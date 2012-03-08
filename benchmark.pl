#!/usr/bin/perl

use strict;
use warnings;

use Time::HiRes;
use Algorithm::SpatialIndex;




sub benchmark {

    my ($idx, $n, $radius) = @_;

    for (0..$n) {

        my $x0 = 1/2 - $radius;
        my $x1 = 1/2 + $radius;

        my $y0 = 1/2 - $radius;
        my $y1 = 1/2 + $radius;

        $idx->get_items_in_rect($x0, $x1, $y0, $y1);

        $radius /= 2;
    }
}



my $n_points  = 500_000;
my $n_splits  = 2;
my $n_tests   = 1_000_000;
my $n_buckets = 200;

my $init_radius = 1/64;




my $idx = Algorithm::SpatialIndex->new(
    strategy    => 'QuadTree',
    storage     => 'Memory',
    limit_x_low => 0,
    limit_x_up  => 1,
    limit_y_low => 0,
    limit_y_up  => 1,
    bucket_size => $n_buckets,
);



for my $i (1..$n_points) {
    $idx->insert($i, rand(), rand());
}


my $start = Time::HiRes::time();
print "start: $start\n";

for (1..$n_tests) {
    benchmark($idx, $n_splits, $init_radius);
}

my $end = Time::HiRes::time();
print "end: $end\n";
my $time = ($end - $start);


print "time: $time\n";
