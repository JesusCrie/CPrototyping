#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

static const char TRANSLATION_TABLE[64] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', // 00-12
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', // 13-25
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', // 26-38
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', // 39-51
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'       // 52-63
};

static const char REVERSE_TABLE[81] = {
//      +               /   0   1   2   3   4   5   6   7   8   9
        62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1,
//              =              A  B  C  D  E  F  G  H  I
        -1, -1, 0, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
//      J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y
        9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
//      Z                           a   b   c   d   e   f   g   h   i
        25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34,
//      j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y
        35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
//      z
        51
};

static const char COMPLEMENT = '=';

static void encode_full_block(const char *data, char *out) {
    out[0] = TRANSLATION_TABLE[data[0] >> 2];
    out[1] = TRANSLATION_TABLE[(data[0] & 0x03) << 4 | (data[1] & 0xf0) >> 4];
    out[2] = TRANSLATION_TABLE[(data[1] & 0x0f) << 2 | (data[2] & 0xc0) >> 6];
    out[3] = TRANSLATION_TABLE[data[2] & 0x3f];
}

static void encode_partial_block(const char *data, char *out, size_t len) {
    // data[0]  data[1]  data[2]
    // 00001111 00001111 00001111
    // 000011 110000 111100 001111
    // out[0] out[1] out[2] out[3]

    out[0] = TRANSLATION_TABLE[data[0] >> 2];
    out[1] = TRANSLATION_TABLE[((data[0] & 0x03) << 4) | ((data[1] & 0xf0) >> 4)];
    out[2] = COMPLEMENT;
    out[3] = COMPLEMENT;

    if (len >= 2) {
        out[2] = TRANSLATION_TABLE[((data[1] & 0x0f) << 2) | ((data[2] & 0xc0) >> 6)];

        if (len == 3) {
            out[3] = TRANSLATION_TABLE[data[2] & 0x3f];
        }
    }
}

static char decode_char(char c) {
    return REVERSE_TABLE[c - '+'];
}

static void decode_full_block(const char *data, char *out) {

    uint32_t block = decode_char(data[0]) << 18
                     | decode_char(data[1]) << 12
                     | decode_char(data[2]) << 6
                     | decode_char(data[3]);

    out[0] = (block & 0xff0000) >> 16;
    out[1] = (block & 0x00ff00) >> 8;
    out[2] = block & 0x0000ff;
}

static void decode_potentially_partial_block(const char *data, char *out) {
    uint32_t block = decode_char(data[0]) << 18
                     | decode_char(data[1]) << 12;

    out[0] = (block & 0xff0000) >> 16;
    out[1] = out[2] = 0;

    if (data[2] != COMPLEMENT) {
        block |= decode_char(data[2]) << 6;
        out[1] = (block & 0x00ff00) >> 8;
    }

    if (data[3] != COMPLEMENT) {
        block |= decode_char(data[3]);
        out[2] = block & 0x0000ff;
    }
}

static char *encode_sequence(const char *data) {
    // compute real len and full block len
    int len = strlen(data);
    int fullBlockLen = len;
    while (fullBlockLen % 3 != 0) {
        --fullBlockLen;
    }

    // allocate space for output (approximately 33% larger than the input)
    char *out = malloc((int) ((double) fullBlockLen * 1.4));
    memset(out, 0, sizeof(&out));

    // encode full blocks
    for (int iIn = 0, iOut = 0; iIn < fullBlockLen; iIn += 3, iOut += 4) {
        encode_full_block(data + iIn, out + iOut);
    }

    // encode last partial block if any
    if (fullBlockLen != len) {
        encode_partial_block(data + fullBlockLen, out + strlen(out), len - fullBlockLen);
    }

    return out;
}

static char *decode_sequence(const char *data) {
    int len = strlen(data);

    // allocate space for output
    char *out = malloc(len);
    memset(out, 0, sizeof(&out));

    // decode full blocks
    for (int iIn = 0, iOut = 0; iIn < len - 4; iIn += 4, iOut += 3) {
        decode_full_block(data + iIn, out + iOut);
    }

    // decode potentially partial block
    decode_potentially_partial_block(data + len - 4, out + strlen(out));

    return out;
}

int main(int argc, char const *argv[]) {
    // Read from stdin
    char *in;
    printf("Enter something: ");
    scanf("%m[^\t\n]", &in);

    // encode to base64
    char *encoded = encode_sequence(in);
    // discard input buffer
    free(in);

    printf("---\n%s\n", encoded);

    // decode from base64
    char *decoded = decode_sequence(encoded);
    // discard encoded buffer
    free(encoded);

    printf("%s\n", decoded);

    // discard remaining buffer
    free(decoded);
    return 0;
}
