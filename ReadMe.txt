usage: 
./lightdes [--ECB/--CTR/--CBC] [--enc/--dec ] [ int rounds ] [ decimal key ] [ if(CBC and enc) [--var/--fix]]

Input: 
Reads input from stdin. 
Writes output to stdout.
Note**: Input reads blocks from left to right. 101 will be read as 101000000000

Example usage:
./lightdes --ECB --enc 10 175 >> output.txt

cat output.txt | ./lightdes --ECB --dec 10 175

./lightdes --CBC --enc 10 175 --var>> output.txt

cat output.txt | ./lightdes --CBC --dec 10 175

./lightdes --CTR --enc 10 175 >> output.txt

cat output.txt | ./lightdes --CTR --dec 10 175

Through makefile:
make
enter binary input to encrypt
encrypts and then decrypts back at you
