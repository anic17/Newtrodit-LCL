existent=0
for x in gcc tdm-gcc tcc
do if dpkg --list | grep compiler | grep -q $x; then existent=1 compiler=$x; fi done
$compiler src/newtrodit.c -o newtrodit -g
