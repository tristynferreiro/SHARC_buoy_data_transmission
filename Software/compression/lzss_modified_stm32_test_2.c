/* 
 * This is a modified version of the original LZSS encoder-decoder (Haruhiko Okumura; public domain) 
 * Instead of taking in a file and array of values is used as input.
 * Instead of printing to a file, the compressed data is store in an array.
 */

#include <stdio.h>
#include <stdlib.h>

#define EI 11  /* typically 10..13 */
#define EJ  5  /* typically 4..5 */
#define P   1  /* If match length <= P then output one character */
#define N (1 << EI)  /* buffer size */
#define F ((1 << EJ) + 1)  /* lookahead buffer size */

int bit_buffer = 0, bit_mask = 128;
unsigned long codecount = 0, textcount = 0;
unsigned char buffer[N * 2];

FILE *infile, *outfile;

/** 
 * This array stores the encoded bit_buffers of all the data. The size needs to be chosen based on the number of bits of data.
 * For the STM32F0 implementation, the size will need to be determine based on available space on the STM. This will likely be 
 * through trial and error
 */
uint32_t compressed[4970000]; // needs to be atleast the size of the input data (minimum). this size should be the limit of data stored at any one time
int compressedBits =0;

/**
* This is the mock input array of data to be compressed
*/
char inputArray[] = "0.054000001,6,0.0024,-0.0006,3.856600046,-0.061000001,-0.061000001,0,34.83589935, 0.054000001,6,0.0024,-0.0006,3.856600046,-0.061000001,-0.061000001,0,34.83589935";

void error(void)
{
    //printf("Output error\n");  
    exit(1);
}

/**
 * This method has been added to store the compression encoded bits in an array that will be passed to the encryption algorithm.
 */
void store(int bitbuffer){
    compressed[compressedBits]=bitbuffer;
    compressedBits++;
}

void putbit1(void)
{
    bit_buffer |= bit_mask;
    if ((bit_mask >>= 1) == 0) {
        store(bit_buffer);
        bit_buffer = 0;  bit_mask = 128;  codecount++;
    }
}

void putbit0(void)
{
    if ((bit_mask >>= 1) == 0) {
        store(bit_buffer);
        bit_buffer = 0;  bit_mask = 128;  codecount++;
    }
}

void flush_bit_buffer(void)
{
    if (bit_mask != 128) {
        store(bit_buffer);
        codecount++;
    }
}

void output1(int c)
{
    int mask;
    
    putbit1();
    mask = 256;
    while (mask >>= 1) {
        if (c & mask) putbit1();
        else putbit0();
    }
}

void output2(int x, int y)
{
    int mask;
    
    putbit0();
    mask = N;
    while (mask >>= 1) {
        if (x & mask) putbit1();
        else putbit0();
    }
    mask = (1 << EJ);
    while (mask >>= 1) {
        if (y & mask) putbit1();
        else putbit0();
    }
}

void encode(void) // should bee modified to take in value
{
    int i, j, f1, x, y, r, s, bufferend, c;
    
    int counter = 0;
    for (i = 0; i < N - F; i++) buffer[i] = ' ';
    for (i = N - F; i < N * 2; i++) {
        if ( counter >= sizeof(inputArray)) break;
        c = inputArray[counter];
        buffer[i] = c;  counter++;
        //textcount++;
    }
    bufferend = i;  r = N - F;  s = 0;
    while (r < bufferend) {
        f1 = (F <= bufferend - r) ? F : bufferend - r;
        x = 0;  y = 1;  c = buffer[r];
        for (i = r - 1; i >= s; i--)
            if (buffer[i] == c) {
                for (j = 1; j < f1; j++)
                    if (buffer[i + j] != buffer[r + j]) break;
                if (j > y) {
                    x = i;  y = j;
                }
            }
        if (y <= P) {  y = 1;  output1(c);  }
        else output2(x & (N - 1), y - 2);
        r += y;  s += y;
        if (r >= N * 2 - F) {
            for (i = 0; i < N; i++) buffer[i] = buffer[i + N];
            bufferend -= N;  r -= N;  s -= N;
            while (bufferend < N * 2) {
                if ( counter >= sizeof(inputArray)) break;
                c = inputArray[counter];
                buffer[bufferend++] = c;  counter++;
                //textcount++;
            }
        }
    }
    
    FILE *f = fopen("testcomp", "w+");
    for (int jk=0;jk<compressedBits;jk++){
        fputc(compressed[jk],f);
        printf("%d\n",compressed[jk]);
    }
    //fprintf(f, "%s",compressed);
    fclose(f);
}
/*
int inputComp[]={152,75,166,19,89,164,193,249,5,49,150,77,165,143,190,25,132,202,105,44,150,191,104,166,15,214,9,156,186,113,53,155,77,159,220,51,71,250,3,250,10,109,49,126,66,128,66,230,15,246,9,163,254,130,103,53,156,78,103,32,80,8};*/
int inputComp[]={ 152,75,166,19,89,164,193,249,5,49,150,77,165,143,190,25,132,202,105,44,150,191,104,166,15,214,9,156,186,113,53,155,77,159,220,51,71,250,3,250,10,109,49,126,66,128,66,230,15,246,9,163,254,130,103,53,156,78,103,32,80,9,99,239,125,255,254,4,13,128};
int lineNumber =0;
int compDataArraySize = 72;

int getbit(int n) /* get n bits */
{
    int i, x;
    static int buf, mask = 0;
    
    x = 0;
    for (i = 0; i < n; i++) {
        if (mask == 0) {
            if (lineNumber>=compDataArraySize) break;
            buf = inputComp[lineNumber];
            mask = 128;
            lineNumber++;
        }
        x <<= 1;
        if (buf & mask) x++;
        mask >>= 1;
    }
    return x;
}

void decode(void)
{
    int i, j, k, r, c;
    
    lineNumber=0;
    
    printf("SIZE: %d",compDataArraySize);
    
    for (i = 0; i < N - F; i++) buffer[i] = ' ';
    r = N - F;
    while ((c = getbit(1)) != EOF) {
        if (c) {
            if (lineNumber >= compDataArraySize) break;
            c=getbit(8);
            printf("%d; %d\n",lineNumber,c);
            fputc(c, outfile);
            buffer[r++] = c;  r &= (N - 1);
        } else {
            i = getbit(EI);
            j = getbit(EJ);
            if (lineNumber>=compDataArraySize) break;
            for (k = 0; k <= j + 1; k++) {
                c = buffer[(i + k) & (N - 1)];
                fputc(c, outfile);
                buffer[r++] = c;  r &= (N - 1);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int enc;
    char *s;
    
    if (argc != 4) {
        printf("Usage: lzss e/d infile outfile\n\te = encode\td = decode\n");
        return 1;
    }
    s = argv[1];
    if (s[1] == 0 && (*s == 'd' || *s == 'D' || *s == 'e' || *s == 'E'))
        enc = (*s == 'e' || *s == 'E');
    else {
        printf("? %s\n", s);  return 1;
    }
    if ((infile  = fopen(argv[2], "rb")) == NULL) {
        printf("? %s\n", argv[2]);  return 1;
    }
    if ((outfile = fopen(argv[3], "wb")) == NULL) {
        printf("? %s\n", argv[3]);  return 1;
    }
    if (enc) encode(); else decode();
    fclose(infile); fclose(outfile);
    return 0;
}