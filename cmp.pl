#! /usr/bin/env perl -n
++$line;
chomp;
s/;/\n/g;
my ($l,$p,$r,@s) = split/\n/;
for my $s (@s){
    die "$line: ERROR: $_\n" unless $s eq $r
}
