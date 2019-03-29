#include <stdio.h>
#include <string.h>
#include <inttypes.h>

static const unsigned char TRANSLATION_TABLE[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', // 00-12
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', // 13-25
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', // 26-38
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', // 39-51
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'       // 51-63
};

static const signed char REVERSE_TABLE[81] = {
//  +               /   0   1   2   3   4   5   6   7   8   9
    62, -1, -1, -1, 63, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, -1,
//                               A   B   C   D   E   F   G   H   I
    -1, -1, -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,
//  J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y
     9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
//  Z                           a   b   c   d   e   f   g   h   i
    25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34,
//  j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
//  z
    51
};

static const unsigned char COMPLEMENT = '=';

static void encode_block(unsigned char* data, unsigned char* out, unsigned int len) {
    // data[0]  data[1]  data[2]
    // 00001111 00001111 00001111
    // 000011 110000 111100 001111
    // out[0] out[1] out[2] out[3]

    uint32_t block = (data[0] << 16)
            | (data[1] << 8)
            | data[2];

    out[0] =            TRANSLATION_TABLE[(block & 0xfc0000) >> 18];
    out[1] =            TRANSLATION_TABLE[(block & 0x03f000) >> 12];
    out[2] = len >= 2 ? TRANSLATION_TABLE[(block & 0x000fc0) >> 6] : COMPLEMENT;
    out[3] = len >= 3 ? TRANSLATION_TABLE[block & 0x000003]        : COMPLEMENT;

    /*out[0] = TRANSLATION_TABLE[data[0] >> 2];
    out[1] = TRANSLATION_TABLE[((data[0] & 0x03) << 4) | ((data[1] & 0xf0) >> 4)];
    out[2] = len >= 2 ? TRANSLATION_TABLE[((data[1] & 0x0f) << 2) | ((data[2] & 0xc0) >> 6)] : COMPLEMENT;
    out[3] = len >= 3 ? TRANSLATION_TABLE[data[2] & 0x3f] : COMPLEMENT;*/
}

static char decode_char(unsigned char c) {
    return REVERSE_TABLE[c - '+'];
}

static void decode_block(unsigned char* data, unsigned char* out) {
    // doesn't really works

    uint32_t block = (data[0] << 18)
            | (data[1] << 12)
            | (data[2] << 6)
            | data[3];

    out[0] =                         decode_char(((data[0]) << 2) | ((data[1]) >> 4));
    out[1] = data[2] != COMPLEMENT ? decode_char(((data[1]) << 4) | ((data[2]) >> 2)) : 0x00;
    out[3] = data[3] != COMPLEMENT ? decode_char(((data[2]) << 6) | ((data[3]) >> 0)) : 0x00;
}

int main(int argc, char const *argv[]) {
    // Prepare memory
    unsigned char in[50];
    unsigned char out[66];
    memset(out, 0, sizeof(out));

    printf("Enter something: ");
    scanf("%[^\t\n]s", in); // vulnerable to stack overflow

    unsigned int len = strlen(in);

    for (int i = 0, y = 0; i < len; i += 3, y += 4) {
        encode_block(in + i, out + y, strlen(in + i));
    }

    printf("---\n%s\n", out);

    for (int i = 0, y = 0; i < len; i += 4, y += 3) {
        decode_block(out + i, in + y);
    }

    printf("%s\n", in);
    return 0;
}
