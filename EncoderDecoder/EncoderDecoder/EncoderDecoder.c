#include <stdio.h>
#include "Encoder.h"
#include "LogHistory.h"

// This two function delete the content that there is in "output.bin" and "log.txt"
// In the end delete this function and the line 31 32
//----------------------------------------------------------------------------------------
void clearFile1(const char* filename) {
    //Open the file in write mode, which will truncate the file
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fclose(file);
}
void clearFile2(const char* filename) {
    // Open the file in write mode, which will truncate the file
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fclose(file);
}
//----------------------------------------------------------------------------------------

int main()
{
    //----------------------------
    clearFile1("output.bin");
    clearFile2("log.txt");
    //----------------------------

	int data = 3;
	encode(&data, sizeof(data) * 8);


	//רק לוודא שהכל פועל כשורה
	readProtectionData();

    //------- logs ------- 
    printLogs();
    //--------------------
}

