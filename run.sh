#!/bin/bash

yacc -d -y 2005084.y
echo 'Generated the parser C file as well the header file'
g++ -w -c -o y.o y.tab.c
echo 'Generated the parser object file'
flex 2005084.l
echo 'Generated the scanner C file'
g++ -w -c -o l.o lex.yy.c
# if the above command doesn't work try g++ -fpermissive -w -c -o l.o lex.yy.c
echo 'Generated the scanner object file'
g++ y.o l.o -lfl -o 2005084
echo 'All ready, running'
./2005084 exp.c