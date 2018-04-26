#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <math.h>

#define FIXED_IV (0b110101101010)

int rounds = 1;

//Expands 6 bits to 8 bits
int expand(int rs){
  int bits[6];
  int i = 0;

  //Gets each bit from the 6 bits of input
  for(i; i < 6; i++){
    int power = pow(2, 5 - i);
    bits[i] = (rs & power) / power;
  }

  int newVal =
  (bits[0] << 7) +
  (bits[1] << 6) +
  (bits[3] << 5) +
  (bits[2] << 4) +
  (bits[3] << 3) +
  (bits[2] << 2) +
  (bits[4] << 1) +
  (bits[5]);
  return newVal;
}

//Prints an integer's first "bits" bits
void printBinary(int num, int bits){
  int i = bits;
  for(i; i >= 1; i--){
    int power = pow(2, i - 1);
    printf("%i", (num & power) / power);
  }
}

//Takes 4 bit input and returns 3 bit sbox value
int sbox(int num, int boxNum){
  int sbox[2][8];
  if (boxNum == 0){
    int sbox1[2][8] = {
      {0b101, 0b010, 0b001, 0b110, 0b011, 0b100, 0b111, 0b000},
      {0b001, 0b100, 0b110, 0b010, 0b000, 0b111, 0b101, 0b011}};
    memcpy(sbox, sbox1, sizeof(sbox));
  }
  else{
    int sbox2[2][8] = {
      {0b100, 0b000, 0b110, 0b101, 0b111, 0b001, 0b011, 0b010},
      {0b101, 0b011, 0b000, 0b111, 0b110, 0b010, 0b001, 0b100}};
    memcpy(sbox, sbox2, sizeof(sbox));
  }

  int row = (num & 0b1000) / 0b1000;
  int col = (num & 0b0111);

  return sbox[row][col];
}

//Takes 12 bits of input(a single block) and performs one round light des with a key
int encrypt (int rs, int key){
  //Expands rs to 8 bits
  int expanded = expand(rs);

  int expandAndKey = expanded^key;
  int ls2 = (expandAndKey & 0b11110000) >> 4; //ls of 8 bit rs xored with key
  int rs2 = (expandAndKey & 0b00001111); //rs of 8 bit rs xored with key

  int sbox1 = sbox(ls2, 0);
  int sbox2 = sbox(rs2, 1);

  int foutput = (sbox1 << 3) + sbox2;

  return foutput;
}

//Encrypts a block using the full light dec functionality with rounds
int lightDesEncrypt(int block, int key){
  int val = block;
  int initialKey = key;
  int i;
  for(i = 0; i < rounds; i++){
    int currentKey = (((initialKey << (i % 9)) + (initialKey >> (9 - (i % 9)))) >> 1) & 0b11111111;
    int ls = (val & 0b111111000000) >> 6;
    int rs = (val & 0b000000111111);

    int encryptedRs = encrypt(rs , currentKey);
    val = (rs << 6) + (encryptedRs^ls);
  }

  return val;
}

//Encrypts a block using the full light dec functionality with rounds
int lightDesDecrypt(int block, int key){
  int val = block;
  int initialKey = key;
  val = ((val << 6) | (val >> 6)) & 0b111111111111;

  int i;
  for(i = rounds - 1; i >= 0; i--){
    int currentKey = (((initialKey << (i % 9)) + (initialKey >> (9 - (i % 9)))) >> 1) & 0b11111111;

    int ls = (val & 0b111111000000) >> 6;
    int rs = (val & 0b000000111111);

    int encryptedRs = encrypt(rs , currentKey);
    val = (rs << 6) + (encryptedRs^ls);
  }

  val = ((val << 6) | (val >> 6)) & 0b111111111111;

  return val;
}

//Performs cbc encrypt on all blocks
int* cipherBlockChainingEncrypt(int *blocks, int totalBlocks, int iv, int key){
  int i;
  for (i = 0; i < totalBlocks; i++) {
    blocks[i] = lightDesEncrypt(blocks[i] ^ iv, key);
    iv = blocks[i];
  }
  return blocks;
}

//Performs cbc decrypt on all blocks
int* cipherBlockChainingDecrypt(int *blocks, int totalBlocks, int iv, int key){
  int i;
  for (i = 0; i < totalBlocks; i++) {
    int nextIV = blocks[i];
    blocks[i] = lightDesDecrypt(blocks[i], key)^iv;
    iv = nextIV;
  }
  return blocks;
}

//Performs ctr encrypt on all blocks
int* ctrEncrypt(int *blocks, int totalBlocks, int ctr, int key){
  int i;
  for (i = 0; i < totalBlocks; i++) {
    blocks[i] = lightDesEncrypt(ctr, key) ^ blocks[i];
    ctr++;
  }
  return blocks;
}

//Performs ctr decrypt on all blocks
int* ctrDecrypt(int *blocks, int totalBlocks, int ctr, int key){
  int i;
  for (i = 0; i < totalBlocks; i++) {
    blocks[i] = lightDesEncrypt(ctr, key) ^ blocks[i];
    ctr++;
  }
  return blocks;
}

//Converts a string to an integer assuming the string is a binary representation
int stringToBlock(char* str){
  int total = 0;
  int i = 0;
  for(i = 0; i < 12; i++){
    if (str[i] == '1'){
      total += 1 << (11 - i);
    } else if(str[i] == '\0'){
      break;
    }
  }
  return total;
}

// /my−cipher [−−ECB/−−CTR/−−CBC] [−−enc/−−dec ] [ int rounds ] [ decimal key ]
int main(int argc, char *argv[]){
  int initialKey = atoi(argv[4]);
  rounds = atoi(argv[3]);

  if (strcmp(argv[2], "--enc") == 0) {
    fclose(fopen("output.txt", "w"));
    char input[360];
    scanf("%s", input);
    int totalBlocks = ceil(strlen(input) / 12.0);
    int blocks[totalBlocks];

    int i;
    for (i = 0; i < strlen(input); i+=12){
      blocks[i / 12] = stringToBlock(&input[i]);
    }

    if (strcmp(argv[1], "--ECB") == 0){
      int j;
      for (j = 0; j < totalBlocks; j++) {
        blocks[j] = lightDesEncrypt(blocks[j], initialKey);
      }
    }
    else if(strcmp(argv[1], "--CTR") == 0){
      int nonce = rand() % 0b11111111;//8 bit long nonce
      ctrEncrypt(blocks, totalBlocks, nonce, initialKey);
      printBinary(nonce, 12);
    }
    else if(strcmp(argv[1], "--CBC") == 0){
      int varIV = rand() % 0b111111111111;//12 bit long varIV
      int iv;
      if (strcmp(argv[5], "--var") == 0){
        iv = varIV;
      }
      else if (strcmp(argv[5], "--fix") == 0){
        iv = FIXED_IV;
      }
      cipherBlockChainingEncrypt(blocks, totalBlocks, iv, initialKey);
      printBinary(iv, 12);
    }
    for (i = 0; i < totalBlocks; i++){
      printBinary(blocks[i], 12);
    }
  }
  else if (strcmp(argv[2], "--dec") == 0){
    char input[360];
    scanf("%s", input);
    int totalBlocks = ceil(strlen(input) / 12.0);
    if(strcmp(argv[1], "--CTR") == 0 || strcmp(argv[1], "--CBC") == 0){
      totalBlocks--;
    }
    int blocks[totalBlocks];
    int nonce; //Is also IV

    int i;
    if(strcmp(argv[1], "--CTR") == 0 || strcmp(argv[1], "--CBC") == 0){
      nonce = stringToBlock(&input[0]);
      for (i = 12; i < strlen(input); i+=12){
        blocks[(i / 12) - 1] = stringToBlock(&input[i]);
      }
    }else{
      for (i = 0; i < strlen(input); i+=12){
        blocks[i / 12] = stringToBlock(&input[i]);
      }
    }

    if (strcmp(argv[1], "--ECB") == 0){
      int j;
      for (j = 0; j < totalBlocks; j++) {
        blocks[j] = lightDesDecrypt(blocks[j], initialKey);
      }
    }
    else if(strcmp(argv[1], "--CTR") == 0){
      ctrDecrypt(blocks, totalBlocks, nonce, initialKey);
    }
    else if(strcmp(argv[1], "--CBC") == 0){
      cipherBlockChainingDecrypt(blocks, totalBlocks, nonce, initialKey);
    }
    for (i = 0; i < totalBlocks; i++){
      printBinary(blocks[i], 12);
    }
  }
  printf("\n");

  return 0;
}
