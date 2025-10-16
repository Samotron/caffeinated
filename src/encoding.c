#include "encoding.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

// Base64 encoding/decoding
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode_data(const unsigned char *data, size_t input_length, size_t *output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = malloc(*output_length + 1);
    if (!encoded_data) return NULL;
    
    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }
    
    int padding = input_length % 3;
    if (padding > 0) {
        for (int i = 0; i < 3 - padding; i++)
            encoded_data[*output_length - 1 - i] = '=';
    }
    
    encoded_data[*output_length] = '\0';
    return encoded_data;
}

int cmd_base64_encode(const char *input, int is_file) {
    unsigned char *data;
    size_t length;
    
    if (is_file) {
        FILE *fp = fopen(input, "rb");
        if (!fp) {
            fprintf(stderr, "Cannot open file: %s\n", input);
            return 1;
        }
        
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        data = malloc(length);
        fread(data, 1, length, fp);
        fclose(fp);
    } else {
        data = (unsigned char*)input;
        length = strlen(input);
    }
    
    size_t output_length;
    char *encoded = base64_encode_data(data, length, &output_length);
    
    if (encoded) {
        printf("%s\n", encoded);
        free(encoded);
    }
    
    if (is_file) free(data);
    return 0;
}

int base64_decode_char(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

int cmd_base64_decode(const char *input, int is_file) {
    const char *data;
    char *file_data = NULL;
    
    if (is_file) {
        FILE *fp = fopen(input, "r");
        if (!fp) {
            fprintf(stderr, "Cannot open file: %s\n", input);
            return 1;
        }
        
        fseek(fp, 0, SEEK_END);
        long length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        file_data = malloc(length + 1);
        fread(file_data, 1, length, fp);
        file_data[length] = '\0';
        fclose(fp);
        data = file_data;
    } else {
        data = input;
    }
    
    size_t input_length = strlen(data);
    size_t output_length = input_length / 4 * 3;
    
    if (data[input_length - 1] == '=') output_length--;
    if (data[input_length - 2] == '=') output_length--;
    
    unsigned char *decoded = malloc(output_length + 1);
    
    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t a = data[i] == '=' ? 0 : base64_decode_char(data[i]);
        i++;
        uint32_t b = data[i] == '=' ? 0 : base64_decode_char(data[i]);
        i++;
        uint32_t c = data[i] == '=' ? 0 : base64_decode_char(data[i]);
        i++;
        uint32_t d = data[i] == '=' ? 0 : base64_decode_char(data[i]);
        i++;
        
        uint32_t triple = (a << 18) + (b << 12) + (c << 6) + d;
        
        if (j < output_length) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < output_length) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < output_length) decoded[j++] = triple & 0xFF;
    }
    
    decoded[output_length] = '\0';
    printf("%s\n", decoded);
    
    free(decoded);
    if (file_data) free(file_data);
    return 0;
}

// UUID v4 generator
int cmd_uuid_generate(int count) {
    srand(time(NULL));
    
    for (int n = 0; n < count; n++) {
        unsigned char uuid[16];
        for (int i = 0; i < 16; i++) {
            uuid[i] = rand() & 0xFF;
        }
        
        // Set version to 4
        uuid[6] = (uuid[6] & 0x0F) | 0x40;
        // Set variant to RFC4122
        uuid[8] = (uuid[8] & 0x3F) | 0x80;
        
        printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
               uuid[0], uuid[1], uuid[2], uuid[3],
               uuid[4], uuid[5],
               uuid[6], uuid[7],
               uuid[8], uuid[9],
               uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    }
    
    return 0;
}
