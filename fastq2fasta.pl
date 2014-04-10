#!/usr/bin/perl
use strict;
&main;

sub main{
    my $line;
    my $i;
    for($i=0; $line=<>; $i++){
        $i = $i % 4;
        if($i == 0){
            $line=~s/^@//;
            print ">$line";
        }elsif($i == 1){
            chomp($line);
            $line=~s/(.{60})/$1\n/g;
            print "$line\n";
        }
    }
}
