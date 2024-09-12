#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// פונקציה לקרוא תוכן של קובץ טקסט לתוך char*
char* read_file_to_string(const char* filename, long* out_length) {
    FILE* file = NULL;
    errno_t err = fopen_s(&file, filename, "rb"); // פתיחת הקובץ במצב קריאה בינארית
    if (err != 0 || file == NULL) {
        perror("Error opening file");
        return NULL;
    }
    // קביעת מיקום הסיום של הקובץ
    fseek(file, 0, SEEK_END);
    long length = ftell(file); // קבלת גודל הקובץ
    fseek(file, 0, SEEK_SET);  // החזרת מיקום הקריאה להתחלה
    if (length < 0) {
        perror("Error determining file size");
        fclose(file);
        return NULL;
    }
    // הקצאת זיכרון עבור התוכן של הקובץ
    char* buffer = (char*)malloc(length + 1); // +1 עבור תו הסיום '\0'
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }
    // קריאת התוכן לקובץ לתוך הזיכרון
    size_t read_size = fread(buffer, 1, length, file);
    if (read_size != length) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[length] = '\0'; // הוספת תו סיום מחרוזת
    fclose(file);
    if (out_length) {
        *out_length = length;
    }
    return buffer;
}
// פונקציה לכתוב char* לתוך קובץ טקסט
int write_string_to_file(const char* filename, const char* data, long length) {
    FILE* file = NULL;
    errno_t err = fopen_s(&file, filename, "wb"); // פתיחת הקובץ במצב כתיבה בינארית
    if (err != 0 || file == NULL) {
        perror("Error opening file");
        return -1;
    }
    size_t written_size = fwrite(data, 1, length, file);
    if (written_size != length) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0; // הצלחה
}



