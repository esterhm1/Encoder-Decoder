
#include "decoder.h"
#include "files.h"



//int main()
//{
//	char a[3]={9,7,7};
//	int len = sizeof(a) * 8;
//	encode(a, len);
//	printf("before ");
//	binary_represent(pd[0].hamming1, NUM_BYTES(HAMMING1_SIZE)*8);
//	change_bit_in_data(pd[0].hamming1, 1);
//	change_bit_in_data(pd[0].hamming1, 2);
//	printf("after  ");
//	binary_represent(pd[0].hamming1, NUM_BYTES(HAMMING1_SIZE)*8);
//	char* decoded=decode(pd, sizeof(a) * 8);
//	binary_represent(decoded, sizeof(a) * 8);
//}


int main() {
    const char* input_filename = "input.png";
    const char* output_filename = "output.png";
    long length;
    char* content = read_file_to_string(input_filename, &length);
    if (content == NULL) {
        return 1; // כישלון בקריאה
    }

    encode(content, length * 8);
    for (size_t i = 1; i <20; i++)
    {
        change_bit_in_data(pd[i].hamming1, 3);
        change_bit_in_data(pd[i].hamming1, 4);
    }
       
 
    char* decoded = decode(pd, length * 8);

    // כתיבה לתוך קובץ חדש
    if (write_string_to_file(output_filename, decoded,length) != 0) {
        free(content);
        return 1; // כישלון בכתיבה
    }

    free(content);
    return 0; // הצלחה
}

