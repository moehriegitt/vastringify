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
    if ($l =~ m(^/[*]\s*(COPY|SNIPPET)\s*(\S+)\s+(.+)[*]/$)) {
        my ($mode, $in_md, $s) = ($1,$2,$3);
        my @md = file_contents($in_md);
        my $in_func = 0;
        my $was_dotdotdot;
        $s =~ s/\s+$//;
        print STDERR "INFO: $mode: Section '$s'\n";
        my $copy = 0;
        my $in_code = 0;
        my $in_comment = 0;
        my $unique_id = 0;
        my $inc = undef;
        LINE: for my $k (@md) {
            if ($copy && ($k =~ /^(\Q$copy\E)\s/)) {
                $copy = 0;
            }
            if ($copy) {
                if ($in_code && ($k =~ /^[.][.][.]/)) {
                    $was_dotdotdot = 1;
                    next LINE;
                }
                if ($inc && ($k =~ /^#include/)) {
                    push @c_out, "/* #include ... */\n";
                    push @$inc, $k;
                    next LINE;
                }
                if ($in_code && ($k =~ /^```/)) {
                    $in_code = 0;
                    if (($mode eq 'SNIPPET') && !$was_dotdotdot) {
                        push @c_out, "}\n";
                        $inc = undef;
                        $in_func = 0;
                    }
                    next LINE;
                }
                if ($in_code || ($k =~ /^    (.*)$/)) {
                    $k = "$1\n" if !$in_code;
                    $in_comment = 0;
                    for my $id (sort keys %enumerate) {
                        $k =~ s/\b\Q$id\E\b/$id$unique_id/g;
                    }
                }
                elsif ($k =~ /^$/) {
                }
                elsif ($k =~ /^```c/i) {
                    $in_code = 1;
                    if ($mode eq 'SNIPPET') {
                        $unique_id++;
                        unless ($in_func) {
                            $inc = [];
                            push @c_out, $inc, "void testfunc$unique_id(void) {\n";
                            $in_func = 1;
                        }
                    }
                    next LINE;
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
                $was_dotdotdot = 0;
                push @c_out, $k;
            }
            if ($k =~ /^(#+)\s+\Q$s\E$/) {
                $copy = $1;
            }
        }
    }
}

open(my $f, '>', "$out_c.new") or die "ERROR: open $out_c.new: $!";
for my $s (@c_out) {
    if (ref($s) eq 'ARRAY') {
        for my $s1 (@$s) {
            print {$f} $s1;
        }
    }
    else {
        print {$f} $s;
    }
}
close($f);

rename "$out_c.new", "$out_c" or die "ERROR: rename $out_c.new: $!";

0;
