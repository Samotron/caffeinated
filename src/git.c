#include "git.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmd_git_stats(const char *repo_path) {
    char cmd[512];
    
    printf("=== Git Repository Statistics ===\n\n");
    
    // Check if git repo
    snprintf(cmd, sizeof(cmd), "cd %s && git rev-parse --git-dir > /dev/null 2>&1", 
             repo_path ? repo_path : ".");
    if (system(cmd) != 0) {
        fprintf(stderr, "Not a git repository\n");
        return 1;
    }
    
    // Repository info
    printf("Repository: ");
    fflush(stdout);
    snprintf(cmd, sizeof(cmd), "cd %s && basename `git rev-parse --show-toplevel`", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    printf("Branch: ");
    fflush(stdout);
    snprintf(cmd, sizeof(cmd), "cd %s && git branch --show-current", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    printf("\n--- Commit Statistics ---\n");
    printf("Total commits: ");
    fflush(stdout);
    snprintf(cmd, sizeof(cmd), "cd %s && git rev-list --count HEAD", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    printf("Contributors: ");
    fflush(stdout);
    snprintf(cmd, sizeof(cmd), "cd %s && git shortlog -sn --all | wc -l", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    printf("\n--- Top Contributors ---\n");
    snprintf(cmd, sizeof(cmd), "cd %s && git shortlog -sn HEAD | head -5", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    printf("\n--- File Statistics ---\n");
    printf("Files tracked: ");
    fflush(stdout);
    snprintf(cmd, sizeof(cmd), "cd %s && git ls-files | wc -l", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    printf("\n--- Recent Activity ---\n");
    printf("Last commit: ");
    fflush(stdout);
    snprintf(cmd, sizeof(cmd), "cd %s && git log -1 --format='%%ar by %%an'", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    printf("\n--- Language Stats ---\n");
    snprintf(cmd, sizeof(cmd), 
             "cd %s && git ls-files | grep -E '\\.(c|cpp|h|py|js|java|go|rs)$' | "
             "sed 's/.*\\.//' | sort | uniq -c | sort -rn | head -5", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    printf("\n--- Lines of Code ---\n");
    snprintf(cmd, sizeof(cmd), 
             "cd %s && git ls-files | xargs wc -l 2>/dev/null | tail -1", 
             repo_path ? repo_path : ".");
    system(cmd);
    
    return 0;
}
