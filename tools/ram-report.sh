#!/bin/sh

if [ "$1" = "-s" ]
then
	nm -S -l -t d --size-sort source/badge2025_c | grep ' [bBgGdD] ' | sed -e 's/:.*$//' |\
	awk '
		{ ram[$5] += $2;   }
		END { for (key in ram) {
			printf("%d %s\n", ram[key], key);
		      }
		}' | sort -n
else
	nm -S -l -t d --size-sort source/badge2025_c | grep ' [bBgGdD] '

	echo
	echo
	echo "Use -s option to get a per .c file summary"
fi
