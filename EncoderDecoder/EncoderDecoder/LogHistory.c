#include "LogHistory.h"
#include <stdio.h>

#define LOG_FILE "log.txt"
#define MAX_LOG_ENTRIES 100
#define LOG_ENTRY_SIZE 32

static char logHistory[MAX_LOG_ENTRIES][LOG_ENTRY_SIZE];
static int logIndex = 0;

void logAction(const char* action) {
    if (logIndex >= MAX_LOG_ENTRIES) {
        return; // Log history is full
    }

    // Store the action in memory
    // snprintf() Used to format and store a string into a buffer- logHistory[][].
    snprintf(logHistory[logIndex], LOG_ENTRY_SIZE, "%s", action);
    logIndex++;

    // Append to log file
    FILE* logFile = fopen(LOG_FILE, "a");
    if (logFile) {
        fprintf(logFile, "%s\n", action);
        fclose(logFile);
    }
}

void printLogs(void) {
    FILE* logFile = fopen(LOG_FILE, "r");
    if (logFile) {
        char line[LOG_ENTRY_SIZE];
        while (fgets(line, sizeof(line), logFile)) {
            printf("%s", line);
        }
        fclose(logFile);
    }
    else {
        printf("Error opening log file.\n");
    }
}