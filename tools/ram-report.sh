#!/bin/sh

if [ "$1" = "-s" ]
then
	nm -S -l -t d --size-sort source/badge2025_c | grep ' [bBgGdD] ' | sed -e 's/:.*$//' |\
	awk '
		BEGIN { total = 0 }
		{
			k = $5;
			if (k == "")
				k = $4 " (static local)";
			ram[k] += $2;
			total += $2;
		}
		END { for (key in ram) {
			printf("%d %s\n", ram[key], key);
		      }
		      printf("%d Total\n", total);
		}' | sort -n
else
	nm -S -l -t d --size-sort source/badge2025_c | grep ' [bBgGdD] '

	echo
	echo
	echo "Use -s option to get a per .c file summary"
fi
