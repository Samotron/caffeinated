#ifndef ENCODING_H
#define ENCODING_H

int cmd_base64_encode(const char *input, int is_file);
int cmd_base64_decode(const char *input, int is_file);
int cmd_uuid_generate(int count);

#endif
