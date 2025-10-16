#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Lorem ipsum generator
static const char *lorem_words[] = {
    "lorem", "ipsum", "dolor", "sit", "amet", "consectetur", "adipiscing", "elit",
    "sed", "do", "eiusmod", "tempor", "incididunt", "ut", "labore", "et", "dolore",
    "magna", "aliqua", "enim", "ad", "minim", "veniam", "quis", "nostrud",
    "exercitation", "ullamco", "laboris", "nisi", "aliquip", "ex", "ea", "commodo",
    "consequat", "duis", "aute", "irure", "in", "reprehenderit", "voluptate",
    "velit", "esse", "cillum", "fugiat", "nulla", "pariatur", "excepteur", "sint",
    "occaecat", "cupidatat", "non", "proident", "sunt", "culpa", "qui", "officia",
    "deserunt", "mollit", "anim", "id", "est", "laborum"
};

static const int lorem_words_count = sizeof(lorem_words) / sizeof(lorem_words[0]);

int cmd_lorem(int words) {
    if (words <= 0 || words > 10000) {
        fprintf(stderr, "Word count must be between 1 and 10000\n");
        return 1;
    }
    
    srand(time(NULL));
    
    int words_in_line = 0;
    for (int i = 0; i < words; i++) {
        const char *word = lorem_words[rand() % lorem_words_count];
        
        if (i == 0) {
            // Capitalize first word
            printf("%c%s", toupper(word[0]), word + 1);
        } else {
            printf(" %s", word);
        }
        
        words_in_line++;
        
        // Add punctuation
        if (words_in_line >= 10 + (rand() % 10)) {
            printf(".");
            if (i < words - 1) {
                printf("\n");
                words_in_line = 0;
            }
        }
    }
    
    if (words_in_line > 0) {
        printf(".\n");
    }
    
    return 0;
}

// Case conversion
int cmd_case_convert(const char *text, const char *to_case) {
    char *result = malloc(strlen(text) + 1);
    if (!result) return 1;
    
    strcpy(result, text);
    
    if (strcmp(to_case, "upper") == 0) {
        for (char *p = result; *p; p++) {
            *p = toupper(*p);
        }
    } else if (strcmp(to_case, "lower") == 0) {
        for (char *p = result; *p; p++) {
            *p = tolower(*p);
        }
    } else if (strcmp(to_case, "title") == 0) {
        int new_word = 1;
        for (char *p = result; *p; p++) {
            if (isspace(*p)) {
                new_word = 1;
            } else if (new_word) {
                *p = toupper(*p);
                new_word = 0;
            } else {
                *p = tolower(*p);
            }
        }
    } else if (strcmp(to_case, "camel") == 0) {
        int new_word = 0;
        char *out = result;
        for (const char *p = text; *p; p++) {
            if (isspace(*p) || *p == '_' || *p == '-') {
                new_word = 1;
            } else {
                if (new_word) {
                    *out++ = toupper(*p);
                    new_word = 0;
                } else {
                    *out++ = tolower(*p);
                }
            }
        }
        *out = '\0';
    } else if (strcmp(to_case, "snake") == 0) {
        char *out = result;
        for (const char *p = text; *p; p++) {
            if (isspace(*p) || *p == '-') {
                *out++ = '_';
            } else {
                *out++ = tolower(*p);
            }
        }
        *out = '\0';
    } else {
        fprintf(stderr, "Unknown case type: %s\n", to_case);
        fprintf(stderr, "Supported: upper, lower, title, camel, snake\n");
        free(result);
        return 1;
    }
    
    printf("%s\n", result);
    free(result);
    return 0;
}
