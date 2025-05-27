#!/bin/sh

for x in *.png
do
	output=`echo $x | sed -e 's/[.]png/.h/' -e 's/[-]/_/g'`
	~/Documents/GitHub/badge2025/tools/png-to-badge-asset 8 $x > $output
done
