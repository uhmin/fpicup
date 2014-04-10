#!/usr/bin/perl
use strict;

&main;

sub main{
    my %options=&interface;
    my %list=&readList(\%options);
    &readFastq(\%options, \%list);
}

sub interface{
    my %options;
    my $i;
    my $number=scalar(@ARGV);
    my $key;
    my $value;
    if($number % 2 == 1){
	&help;
    }
    $options{"-r"}="F";
    $options{"inverse"}=0;
    for($i=0; $i<$number; $i+=2){
	$key=$ARGV[$i];
	$value=$ARGV[$i+1];
	if($key eq "-i" || $key eq "-l" || $key eq "-k" || $key eq "-r"){
	    $options{$key}=$value;
	}else{
	    print stderr "Unknown option: $key\n";
	    &help;
	}
    }

    if(exists($options{"-l"}) && exists($options{-k})){
	print stderr "Options -l and -k cannot be specified at once.\n";
	print stderr "Please select one of these options.\n";
	&help;
    }elsif(!exists($options{"-l"}) && !exists($options{-k})){
	print stderr "Please specify either -l or -k option.\n";
	&help;
    }
    if($options{"-r"} eq "F"){
	$options{"inverse"}=0;
    }elsif($options{"-r"} eq "T"){
	$options{"inverse"}=1;
    }else{
	print stderr "Please select T or F for -r option\n";
	&help;
    }
    return %options;
}

sub help{
    my $file=__FILE__;
    print stderr << "EOF";
$file: extract specified sequences from 
four-lines-per-sequence formatted fastq file.

--- Usage ---
 Examples
$file -i [fastq file] -l [list file]
$file -i [fastq file] -k [sequence ID(s)]
cat [fastq file] | $file -l [list file]
zcat [fastq file gz] | $file -k [sequence ID(s)]

  -i Fastq formatted file. If not specified <stdin> will be selected.
  -l Sequence ID list file. Should be delimited by new lines.
  -k Sequence IDs. Should be delimited by comma.
  -r Reverse list [T/F]. If True, output NOT listed sequences.
     Default is F.

EOF
    ;
    exit(0);
}

sub readList{ #(\%options);
    my $options=$_[0];
    my %list;
    if(exists($options->{-l})){
	%list=&reaad_from_list_file($options);
    }else{
	%list=&read_from_options($options);
    }
    return %list;
}

sub reaad_from_list_file{ #($options);
    my $options=$_[0];
    my %list;
    my $file=$options->{"-l"};
    my $line;
    
    open FIN, "$file" or die("Could not open list file: $file\n");
    while($line=<FIN>){
	chomp($line);
	$line=~s/^@//;
	$line=~s/\s.*//;
	if($line ne ""){
	    $list{$line}=1;
	}
    }
    close FIN;
    return %list;
}

sub read_from_options{ #($options);
    my $options=$_[0];
    my %list;
    my $key=$options->{"-k"};
    my @keys=split(/,/, $key);
    foreach $key (@keys){
	$list{$key}=1;
    }
    return %list;
}

sub readFastq{ #(\%options, \%list);
    my $options=$_[0];
    my $list=$_[1];
    my $file;
    my $line;
    my $key;
    my $sw;
    my $i=0;
    *FIN=*STDIN;
    if(exists($options->{"-i"})){
	$file=$options->{"-i"};
	open FIN, "$file" or die("Could not open infile: $file\n");
    }
    for($i=0; $line=<FIN>; $i++){
	$i=$i % 4;
	if($i==0 && $line=~/^@/){
	    ($key=$line)=~s/^@(\S+)\s.*/$1/;
	    chomp($key);
	    if(exists($list->{$key})){
		$sw=0;
	    }else{
		$sw=1;
	    }
	}
	if($sw==$options->{"inverse"}){
	    print $line;
	}
    }
    close FIN;
}

