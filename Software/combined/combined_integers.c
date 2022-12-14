/**
**************************************************
Info:		encryption (RSA) and compresstion (lzss)
Author:		Tristyn Ferreiro and Shameera Cassim
****************************************************
This code compresses and encrypts data in a hard coded array and prints the result 
to a file (as INTEGER VALUES). The encryption uses a FIXED KEY.

The compression algorithm uses a modified version of (Haruhiko Okumura; public domain)'s 
lzss encoder. Encryption is based off of AES encryption. Modifications to both of these 
algorithms have been made to suite the desing requirements of this project.
******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//For compression:
#define EI  6  /* typically 10..13 */
#define EJ  5  /* typically 4..5 */
#define P   1  /* If match length <= P then output one character */
#define N (1 << EI)  /* buffer size */
#define F ((1 << EJ) + 1)  /* lookahead buffer size */

int bit_buffer = 0, bit_mask = 128;


//For encryption:
#define MAX_VALUE 32
#define E_VALUE 3 /*65535*/

char inputData [120];

int encryptedData[40];
int encryptedBits = 0;
int e = E_VALUE;
int n = 187;
int d = 107;
int p = 11;
int q = 17;

/**
 * This array stores the encoded bit_buffers of all the data. The size needs to be chosen based on the 
 * number of bits of data. For the STM32F0 implementation, the size will need to be determine based on 
 * available space on the STM. This will likely be through trial and error.
 */
char compressed[200]; // should be at leas half the size of the inout data.
int compressedBits =0;

FILE *outfile;

/***************
 * COMPRESSION *
 ***************/

/**
 * This method has been added to store the compression bits in one array for printing/transmission.
 */
void store(int bitbuffer){
    compressed[compressedBits]=bitbuffer;
    //printf("Out: %d\n",compressed[compressedBits]);
    compressedBits++;
}

void putbit1(void)
{
    bit_buffer |= bit_mask;
    if ((bit_mask >>= 1) == 0) {
        store(bit_buffer);
        bit_buffer = 0;  bit_mask = 128;  
    }
}

void putbit0(void)
{
    if ((bit_mask >>= 1) == 0) {
        store(bit_buffer);
        bit_buffer = 0;  
        bit_mask = 128;  
    }
}

void flush_bit_buffer(void)
{
    if (bit_mask != 128) {
        store(bit_buffer);
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

void compress(void)
{
    int i, j, f1, x, y, r, s, bufferend, c;
    int counter = 0;
    int buffer[N * 2];
    
    for (i = 0; i < N - F; i++) buffer[i] = ' ';
    for (i = N - F; i < N * 2; i++) {
        if (counter > encryptedBits) break;
        c = encryptedData[counter];
        buffer[i] = c;  counter++;
        //printf("buffer = %d\n",buffer[i]);
        //printf("c = %d\n", c);;
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
                if (counter > encryptedBits) break;
                c = encryptedData[counter];
                buffer[bufferend++] = c;  counter++;
            }
        }
    }

    // WRITE compressed bits to FILE
    for (int jk=0;jk<compressedBits;jk++){
        fprintf(outfile,"%d\n",compressed[jk]);
    }
    
}

/**************
 * ENCRYPTION *
 **************/
int ENCmodpow(int base, int power, int mod)
{
        int i;
        int result = 1;
        for (i = 0; i < power; i++)
        {
                result = (result * base) % mod;
        }
        return result;
}

void encrypt(char msg[]) {
    int c;
	int i;
        for (i = 0; msg[i]!= '}'; i++)
        {
            c = ENCmodpow(msg[i],e,n);
            encryptedData[i] = c;
            encryptedBits++;
            //int mesg[4];
           // if (i > 0) {
           // sprintf(mesg, "%d and i-1 =%dP",encryptedData[i], encryptedData[i-1]);
           //  HAL_UART_Transmit(&huart2, mesg, sizeof(mesg), 1000);
           // }
        }
    /*
    for(int i =0; i<encryptedBits;i++){
        printf("%d\n",encryptedData[i]);
    }
    */
    printf("BITS:%d",encryptedBits);
    //Call compression
    compress();
}



int main(int argc, char *argv[])
{
    int enc;
    int dec;
    char *s;

    char input[] = {"-0.28,-0.51,0.32,2.47,-8.75,11.012\n-0.28,-0.51,0.32,2.47,-8.75,11.012}"};
    if (argc != 3) {
        printf("Usage: combined e outfile\n\te = encrypt and compress\n");
        return 1;
    }
    s = argv[1];

    if (s[1] == 0 && (*s == 'e' || *s == 'E')) {
        enc = (*s == 'e' || *s == 'E');
    }
   else {
       printf("? %s\n", s);  return 1;
   }
    if ((outfile = fopen(argv[2], "w")) == NULL) {
        printf("? %s\n", argv[2]);  return 1;
    }
    if (enc) {encrypt(input);}
    fclose(outfile);
    return 0;
}
