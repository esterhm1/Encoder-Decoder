#pragma once

//#ifdef BUILD_MAIN
#include "files.h"
#include "encoder.h"
#include "decoder.h"
int compare_memory(const char* ptr1, const char* ptr2, size_t num) {
    for (size_t i = 0; i < num; i++) {
        if (ptr1[i] != ptr2[i]) {
            return 0;
        }
    }
    return 1;
}
int random_in_range(int x, int y) {
    srand(time(NULL));
    return rand() % (y - x + 1) + x;
}
int main() {
    //const char* input_filename = "C:/Users/USER/Pictures/תמונות אלבום/תמונות לבחור/IMG_1864.JPG";
    //const char* output_filename = "output.txt";
    //long length;
    //char* content = read_file_to_string(input_filename, &length);
    //if (content == NULL) {
    //    return 1; // כישלון בקריאה
    //}

    //encode(content, length * 8);
  /*  for (size_t i = 1; i < 2; i++)
    {
        change_bit_in_data(pd[i].hamming1, 3);
        change_bit_in_data(pd[i].hamming1, 4);
    }*/


    //char* decoded = decode(pd, length * 8);

    //// כתיבה לתוך קובץ חדש
    //if (write_string_to_file(output_filename, decoded, length) != 0) {
    //    free(content);
    //    return 1; // כישלון בכתיבה
    //}

    //free(content);
    int place = random_in_range(1, HAMMING1_SIZE - 2);
    int data[5] = { 1,2,3,4,5 };
    encode(data, sizeof(data) * 8);
    change_bit_in_data(pd[0].hamming1, place);
    change_bit_in_data(pd[0].hamming1, place + 1);
    char* decoded = decode(pd, sizeof(data) * 8);
    printf("%d", (compare_memory((char*)data, decoded, sizeof(data))));
    return 0; // הצלחה
}
//#endif