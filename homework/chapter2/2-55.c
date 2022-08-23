#include <stdio.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len) {
    int i;
    for (i = 0; i < len; i++)
    printf(" %.2x", start[i]);
    printf("\n");
}

void show_int(int x) {
    show_bytes((byte_pointer) &x, sizeof(int));
}

void show_float(float x) {
    show_bytes((byte_pointer) &x, sizeof(float));
}

void show_short(short x) {
    show_bytes((byte_pointer) &x, sizeof(short));
}

void show_long(long x) {
    show_bytes((byte_pointer) &x, sizeof(long));
}

void show_double(double x) {
    show_bytes((byte_pointer) &x, sizeof(double));
}

void show_pointer(void *x) {
    show_bytes((byte_pointer) &x, sizeof(void *));
}

int is_little_endian() {
    short a = 1;
    char *b = (char *)(&a);
    return b[0] == 0x1;
}

unsigned replace_byte(unsigned x, int i, unsigned char b) {
    unsigned t = (unsigned)b;
    for(int j = 0; j < i; ++j){
        t <<= 8;
    }
    unsigned mask;
    switch (i) {
        case 0 : mask = 0xFFFFFF00;
                 break;
        case 1 : mask = 0xFFFF00FF;
                 break;
        case 2 : mask = 0xFF00FFFF;
                 break;
        case 3 : mask = 0x00FFFFFF;
                 break;
        default : return -1;
    };
    x &= mask;
    return x |= t;
}

int main(int argc, char const *argv[])
{
    // int a = 0x88665544;
    // show_int(a);
    // double b = 0x8899aabb00990011;
    // float c = 0x88665544;
    // show_float(c);
    // show_double(b);
    // printf("is_little_endian: %d", is_little_endian());
    printf("result: %.2x", replace_byte(0x12345678, 2, 0xAB));
    return 0;
}
