#! /usr/bin/env perl -n
chomp;
s/;/\n/g;
my ($l,$p,$r,@s) = split/\n/;
for my $s (@s){
    die "ERROR: $_\n" unless $s eq $r
}
