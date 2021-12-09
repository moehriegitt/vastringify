#! /usr/bin/env perl

use strict;
use warnings;

sub file_contents($)
{
    my ($fn) = @_;
    open(my $f, '<', $fn) or die "ERROR: open $fn: $!";
    return <$f>;
}

my ($in_c, $out_c) = @ARGV;
die unless defined $in_c;
die unless defined $out_c;

my %enumerate = ();

my @c_in  = file_contents($in_c);

my @c_out = ();
for my $l (@c_in) {
    if ($l =~ m(^/[*]\s*ENUMERATE:\s*(\S+)\s*[*]/$)) {
        print STDERR "DEBUG: enumerate $1\n";
        $enumerate{$1} = 1;
    }
    push @c_out, $l;
    if ($l =~ m(^/[*]\s*COPY\s*(\S+)\s+(.+)[*]/$)) {
        my ($in_md, $s) = ($1,$2);
        my @md = file_contents($in_md);
        $s =~ s/\s+$//;
        print STDERR "DEBUG: section: $s\n";
        my $copy = 0;
        my $in_comment = 0;
        my $unique_id = 0;
        for my $k (@md) {
            if ($copy && ($k =~ /^(\Q$copy\E)\s/)) {
                $copy = 0;
            }
            if ($copy) {
                if ($k =~ /^    (.*)$/) {
                    $k = "$1\n";
                    $in_comment = 0;
                    for my $id (sort keys %enumerate) {
                        $k =~ s/\b\Q$id\E\b/$id$unique_id/g;
                    }
                }
                elsif ($k =~ /^$/) {
                }
                else {
                    if (!$in_comment) {
                        $in_comment = 1;
                        $unique_id++;
                    }
                    chomp $k;
                    $k =~ s([*]/)(* /)g;
                    $k =~ s(/[*])(/*)g;
                    $k = "/* $k */\n";
                }
                push @c_out, $k;
            }
            if ($k =~ /^(#+)\s+\Q$s\E$/) {
                $copy = $1;
            }
        }
    }
}

open(my $f, '>', "$out_c.new") or die "ERROR: open $out_c.new: $!";
print {$f} @c_out;
close($f);

rename "$out_c.new", "$out_c" or die "ERROR: rename $out_c.new: $!";

0;
