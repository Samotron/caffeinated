#ifndef UTILS_H
#define UTILS_H

int cmd_env(const char *var);
int cmd_env_inspect(const char *env_file);
int cmd_passgen(int length, int flags);
int cmd_findlarge(const char *path, unsigned long long min_size_mb);

// Password generator flags
#define PASS_ALPHA     (1 << 0)
#define PASS_UPPER     (1 << 1)
#define PASS_NUMBERS   (1 << 2)
#define PASS_SPECIAL   (1 << 3)
#define PASS_ALL       (PASS_ALPHA | PASS_UPPER | PASS_NUMBERS | PASS_SPECIAL)

#endif
