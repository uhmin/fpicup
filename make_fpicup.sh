gcc -Wall -O -D_FILE_OFFSET_BITS=64 -o fpicup_linux.5.2.exe        fpicup5.2.100420.c
gcc -Wall -O -D_FILE_OFFSET_BITS=64 -o fpicmakeindex_linux.exe     fpicmakeindex.100618.c
gcc -Wall -O -D_FILE_OFFSET_BITS=64 -o fpicmakebindex4.1_linux.exe fpicmakebindex4.1.2.c
ln -s fpicmakeindex_linux.exe fpicmakeindex.exe
ln -s fpicmakebindex4.1_linux.exe fpicmakebindex4.exe
ln -s fpicup_linux.5.2.exe fpicup.exe
