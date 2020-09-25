#!/bin/sh



QSB_LOCATION="qsb.exe"
QSB_ARGS="--glsl \"150,120,100 es\" -c --msl 12 --hlsl 50 -o"

for i in *.frag *.vert
do
	cmd="${QSB_LOCATION} ${QSB_ARGS} ${i}.qsb ${i}"
	echo "Compiling ${i}"
	eval ${cmd}
done

