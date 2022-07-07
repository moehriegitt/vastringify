#! /usr/bin/env perl
# Roughly computes stack usage.
# Works only on some x86 .s files.
# Produces a .dot file with results.

use strict;
use warnings;
use Data::Dumper;
use Carp;

sub file_contents($)
{
    my ($fn) = @_;
    open(my $f, '<', $fn) or die "ERROR: $fn: open: $!\n";
    return <$f>;
}

sub add_stack($$);
sub add_stack($$)
{
    my ($funs, $fun) = @_;
    my $f = $funs->{$fun} or die;
    $f->{rec} //= 0;
    return if $f->{rec} == 2;
    die "Error: Recursion detected in '$fun'\n" if $f->{rec} == 1;
    croak if $f->{rec};
    $f->{rec} = 1;

    # first add up called functions:
    for my $other ((keys %{ $f->{call} }), (keys %{ $f->{jmp} })) {
        add_stack($funs, $other);
    }

    # get max of all tail called functions:
    $f->{stack}{sum} = $f->{stack}{single};
    $f->{stack}{sum_src} = { $fun => 1 };
    for my $other (keys %{ $f->{jmp} }) {
        my $g = $funs->{$other};
        my $g_sum = $g->{stack}{sum} or die "$fun: ERROR: No stack usage for '$other'\n";
        if ($g_sum > $f->{stack}{sum}) {
            $f->{stack}{sum} = $g_sum;
            $f->{stack}{sum_src} = { $other => 1 };
        }
    }
    for my $other (keys %{ $f->{call} }) {
        my $g = $funs->{$other};
        my $g_sum = $g->{stack}{sum} or die "$fun: ERROR: No stack usage for '$other'\n";
        $g_sum += $f->{stack}{single};
        if ($g_sum > $f->{stack}{sum}) {
            $f->{stack}{sum} = $g_sum;
            $f->{stack}{sum_src} = { $fun => 1, $other => 1 };
        }
        elsif ($g_sum == $f->{stack}{sum}) {
            $f->{stack}{sum_src}{$other} = 1;
        }
    }

    $f->{rec} = 2;
}

my ($in_s, $in_su) = @ARGV;
die "Usage: stack.pl FILE.s [FILE.su]\n" unless defined $in_s;

if (!defined($in_su)) {
    $in_su = $in_s;
    $in_su =~ s/[.]s$//g;
    $in_su .= ".su";
}

my $out_dot = $in_s;
$out_dot =~ s/[.]s$//g;
$out_dot .= ".dot";

my %fun = ();
for my $l (file_contents $in_su) {
    if ($l =~ /^.*:(\S+)\s+(\d+)\s+\S+\n/) {
        my ($fun, $usage) = ($1,$2);
        $fun{$fun}{stack}{single} = $usage;
    }
}

my %possible_call = ();
my $cur = undef;
for my $l (file_contents $in_s) {
    if ($l =~ /^\s*[.]type\s+([.a-zA-Z0-9_]+),\s*\@function\s*$/) {
        my ($fun) = ($1);
        print "fun: $fun\n";
        $fun{$fun} //= {};
    }
    if ($l =~ /^\s*[.]cfi_endproc/) {
        $cur = undef;
    }
    if ($l =~ /^([.a-zA-Z0-9_]+):/) {
        my ($fun) = ($1);
        if (my $f = $fun{$fun}) {
            $cur = $f;
        }
    }
    if ($cur) {
        if ($l =~ /^\s*(call|jmp)\s*([.a-zA-Z0-9_]+)\s*$/) {
            my ($what, $other) = ($1,$2);
            if ($fun{$other}) {
                $cur->{$what}{$other} = 1;
            }
        }
        elsif ($l =~ /^\s*(call|jmp)\s+[*].*\%/) {
            my ($what) = ($1);
            for my $other (keys %possible_call) {
                if ($fun{$other}) {
                    $cur->{$what}{$other} = 1;
                }
            }
            %possible_call = ();
        }
        elsif ($l =~ /possible_call:\s*\"?([a-z_0-9]+)/) {
            $possible_call{$1} = 1;
        }
    }
}

for my $fun (keys %fun) {
    add_stack(\%fun, $fun);
}

open(my $h, '>', $out_dot) or die "ERROR: $out_dot: open: $!\n";
print {$h} "digraph x {\n";
for my $fun (sort keys %fun) {
    my $f = $fun{$fun};
    print {$h} "\"$fun\" [label=\"$f->{stack}{single}\\n$fun\\n$f->{stack}{sum}\"];\n";
    for my $other (sort keys %{ $f->{call} }) {
        my $bold = $f->{stack}{sum_src}{ $other };
        my $bold_str = $bold ? ",style=bold" : ",style=dashed";
        print {$h} "\"$fun\" -> \"$other\" [color=red$bold_str];\n";
    }
    for my $other (sort keys %{ $f->{jmp} }) {
        my $bold = $f->{stack}{sum_src}{ $other };
        my $bold_str = $bold ? ",style=bold" : ",style=dashed";
        print {$h} "\"$fun\" -> \"$other\" [color=black$bold_str];\n";
    }
}
print {$h} "}\n";
close $h;


0;
