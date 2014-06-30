#!/bin/bash
myDIR=$(cd $(dirname $0) && pwd)
$myDIR/fpicmakeindex.exe ${1}
$myDIR/fpicmakebindex4.exe < $1.index >$1.bindex
