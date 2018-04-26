all:
	clear
	gcc -o lightDes lightDes.c -lm
	./lightDes --ECB --enc 10 127 >> output.txt
	cat output.txt | ./lightDes --ECB --dec 10 127
