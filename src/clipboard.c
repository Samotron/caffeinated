#include "clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

int cmd_clipboard_get(void) {
    if (!OpenClipboard(NULL)) {
        fprintf(stderr, "Failed to open clipboard\n");
        return 1;
    }
    
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL) {
        printf("Clipboard is empty or doesn't contain text\n");
        CloseClipboard();
        return 1;
    }
    
    char *pszText = (char *)GlobalLock(hData);
    if (pszText != NULL) {
        printf("%s", pszText);
        GlobalUnlock(hData);
    }
    
    CloseClipboard();
    return 0;
}

int cmd_clipboard_set(const char *text) {
    if (!OpenClipboard(NULL)) {
        fprintf(stderr, "Failed to open clipboard\n");
        return 1;
    }
    
    EmptyClipboard();
    
    size_t len = strlen(text) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hMem == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        CloseClipboard();
        return 1;
    }
    
    memcpy(GlobalLock(hMem), text, len);
    GlobalUnlock(hMem);
    
    if (SetClipboardData(CF_TEXT, hMem) == NULL) {
        fprintf(stderr, "Failed to set clipboard data\n");
        GlobalFree(hMem);
        CloseClipboard();
        return 1;
    }
    
    CloseClipboard();
    printf("Text copied to clipboard\n");
    return 0;
}

#elif defined(__linux__)
#include <unistd.h>

int cmd_clipboard_get(void) {
    // Try xclip first, then xsel
    if (system("which xclip > /dev/null 2>&1") == 0) {
        return system("xclip -selection clipboard -o");
    } else if (system("which xsel > /dev/null 2>&1") == 0) {
        return system("xsel --clipboard --output");
    } else {
        fprintf(stderr, "Clipboard access requires xclip or xsel\n");
        fprintf(stderr, "Install with: sudo apt-get install xclip\n");
        return 1;
    }
}

int cmd_clipboard_set(const char *text) {
    if (system("which xclip > /dev/null 2>&1") == 0) {
        FILE *fp = popen("xclip -selection clipboard", "w");
        if (!fp) {
            fprintf(stderr, "Failed to open xclip\n");
            return 1;
        }
        fprintf(fp, "%s", text);
        pclose(fp);
        printf("Text copied to clipboard\n");
        return 0;
    } else if (system("which xsel > /dev/null 2>&1") == 0) {
        FILE *fp = popen("xsel --clipboard --input", "w");
        if (!fp) {
            fprintf(stderr, "Failed to open xsel\n");
            return 1;
        }
        fprintf(fp, "%s", text);
        pclose(fp);
        printf("Text copied to clipboard\n");
        return 0;
    } else {
        fprintf(stderr, "Clipboard access requires xclip or xsel\n");
        fprintf(stderr, "Install with: sudo apt-get install xclip\n");
        return 1;
    }
}

#else
int cmd_clipboard_get(void) {
    fprintf(stderr, "Clipboard not supported on this platform\n");
    return 1;
}

int cmd_clipboard_set(const char *text) {
    fprintf(stderr, "Clipboard not supported on this platform\n");
    return 1;
}
#endif
