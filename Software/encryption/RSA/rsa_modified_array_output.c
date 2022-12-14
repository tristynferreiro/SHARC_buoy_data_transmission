// modified from code available at https://github.com/yigitusta/RSA-Implementation
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#define MAX_VALUE 65535

#define E_VALUE 3 /*65535*/

uint16_t e = E_VALUE, p, q;
uint32_t n, phi, d;
uint64_t encryptedData[20000];

FILE *infile, *outfile;
uint32_t findD(uint16_t e, uint32_t phi)
{
	uint32_t eprev, dprev, d = 1, etemp, dtemp;

	eprev = phi, dprev = phi;
	while (e != 1)
	{
		etemp = e;
		dtemp = d;
		e = eprev - eprev / etemp * e;
		d = dprev - eprev / etemp * d;
		eprev = etemp;
		dprev = dtemp;
		while (d < 0)
			d += phi;
	}

	return d;
}

int ifprime(uint16_t n)
{
	uint16_t i;
	for (i = 2; i <= n / 2; i++)
	{
		if (n % i == 0)
			return 0;
	}
	return 1;
}

uint16_t gcd(uint16_t num1, uint32_t num2)
{
	uint16_t i, temp;
	if (num1 > num2)
	{
		temp = num1;
		num1 = num2;
		num2 = temp;
	}
	for (i = num1; i > 0; i--)
	{
		if (num1 % i == 0 && num2 % i == 0)
			return i;
	}
}

uint16_t getprime()
{
	uint16_t n;
	do
	{
		srand(time(NULL));
		n = rand() % MAX_VALUE + 5;
	}while  (!ifprime(n));
	return n;
}

void setprimes(uint16_t e, uint16_t *p, uint16_t *q, uint32_t *n, uint32_t *phi)
{
	do
	{
		*p = getprime();
		do
			*q = getprime();
		while(*p == *q);

		*n = *p * *q;
		*phi = *n - *p - *q + 1;
	}while (gcd(e,*phi) != 1);
}

void rsa_init()
{
	setprimes(e, &p, &q, &n, &phi);

	d = findD(e,phi);

	FILE *outp = fopen("public.txt", "w");
	fprintf(outp, "%"PRIu32" %"PRIu16, n, e);
	fclose(outp);

	outp = fopen("private.txt", "w");
	fprintf(outp, "%"PRIu32" %"PRIu32, n, d);
	fclose(outp);

	outp = fopen("pq.txt", "w");
    fprintf(outp, "%"PRIu16" %"PRIu16, p, q);
    fclose(outp);
}

unsigned long long int ENCmodpow(int base, int power, int mod)
{
        int i;
        unsigned long long int result = 1;
        for (i = 0; i < power; i++)
        {
                result = (result * base) % mod;
        }
        return result;
}

void encrypt2(char msg[]) {
    rsa_init();
    int m, n, e;
    unsigned long long int c;

    FILE *inp = fopen("public.txt", "r");
    fscanf(inp, "%d %d", &n, &e);
    fclose(inp);

	int i;
	int elements = sizeof(&msg);
	unsigned long long int temp[elements];

    for (i = 0; msg[i]!= '}'; i++)
    {
        c = ENCmodpow(msg[i],e,n);
        encryptedData[i] = c;
    }
}

void readFromFile(FILE* file, char* arr[]) {
    int wordCount = 0, i;
    //Actions
    if(file != NULL) {
    while(!feof(file)) {
        fscanf(file, "%c", &arr[wordCount]);
        wordCount++;
    }
}
}
void encrypt(FILE* file) {
    char* arr[90000];
    readFromFile(file, arr);
    //encrypt2(arr);
}

unsigned long long int DECmodpow(unsigned long long int base, int power, int mod)
{
        int i;
        unsigned long long int result = 1;
        for (i = 0; i < power; i++)
        {
                result = (result * base) % mod;
        }
        return result;
}

int inverse(int a, int mod)     /*find inverse of number a in % mod*/
{                               /*inverse of a = i*/
        int aprev, iprev, i = 1, atemp, itemp;

        aprev = mod, iprev = mod;
        while (a != 1)
        {
                atemp = a;
                itemp = i;
                a = aprev - aprev / atemp * a;
                i = iprev - aprev / atemp * i;
                aprev = atemp;
                iprev = itemp;
                while (i < 0)
                        i += mod;
        }

        return i;
}


void decrypt() {
        int d, n, p, q, h, m, qInv, m1m2;
        unsigned long long int c, dP, dQ, m1, m2;
        FILE *inp, *out;

        inp = fopen("private.txt", "r");
        fscanf(inp, "%d %d", &n, &d);
        fclose(inp);

        inp = fopen("pq.txt", "r");
        fscanf(inp, "%d %d", &p, &q);
        fclose(inp);

	while (fscanf(infile, "%llu", &c) != EOF)
	{
        	dP = d % (p - 1);
        	dQ = d % (q - 1);
        	qInv = inverse(q,p);
        	m1 = DECmodpow(c,dP,p);
        	m2 = DECmodpow(c,dQ,q);
        	m1m2 = m1 - m2;
        	if (m1m2 < 0)
                	m1m2 += p;
       		h = (qInv * m1m2) % p;
        	m = m2 + h * q;
        	fprintf(outfile, "%c", m);
	}
	fclose(infile);
	fclose(outfile);

}

int main(int argc, char *argv[])
{
    int enc;
    int dec;
    char *s;
    char c[] = "0.054000001,6,0.0024,-0.0006,3.856600046,-0.061000001,-0.061000001,0,34.83589935\n0.066,7,0.0048,-0.003,4.239200115,0,-0.061000001,0,34.84180069\n";

//    if (argc != 4) {
//        printf("Usage: RSA e/d infile outfile\n\te = encode\td = decode\n");
//        return 1;
//    }
    s = argv[1];
    if (s[1] == 0 && (*s == 'd' || *s == 'D' || *s == 'e' || *s == 'E')) {
        enc = (*s == 'e' || *s == 'E');
        dec = (*s == 'd' || *s == 'D');
    }
   else {
       printf("? %s\n", s);  return 1;
   }
    if ((infile  = fopen(argv[2], "rb")) == NULL) {
       printf("? %s\n", argv[2]);  return 1;
    }
    if ((outfile = fopen(argv[3], "w")) == NULL) {
        printf("? %s\n", argv[3]);  return 1;
    }
    if (enc) {encrypt2(c);}
    else if(dec) {decrypt();}
    fclose(infile);  fclose(outfile);
    return 0;
}

