fpicup
======

A C program that picks up sequence(s) from multifasta sequences.

This program will fetch full/partial sequence(s) quickly from multifasta file.
Before use this program, you have to make index files by typing

fpicupmakeindex.sh [FILENAME]

usage:
      -i: Infile name. stdin if default.
      -o: Outfile name. stdout if default.
      -k: Keyword of sequence searching.
      -r: Region to pic up (from)-(to). All sequence ( 0-0 ) if default.
      -w: Letters per line. Display in one line if 0. Zero if default.
      -s: extract oligofasta from multifasta. to get from M-th to N-th sequence [-s M-N] (M <= N)
          to count number of sequence in amultifasta file use only -i option
      -l: File name of query List.

# To picup 100-20 region (this mean reverse complement of 20-100) of
# "sequencename" sequence from "FILENAME" file.
fpicup -i FILENAME -k "sequencename" -r 100-20

# To picup multi sequences listed in LIST file.
fpicup -i FILENAME -l LIST

>cat LIST
sequencename1
sequencename2
sequencename3
sequencename4
sequencename5
>

