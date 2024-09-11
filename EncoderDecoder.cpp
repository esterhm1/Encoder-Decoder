
extern "C" {
#include "decoder.h"
//#include "files.h"
}


int main()
{
	int data[4] = { 1,2,3,4};
	binary_represent(&data, sizeof(data) * 8);
	encode(&data, sizeof(data) * 8);
	char* decoded = decode("output.bin",sizeof(data) * 8);
	binary_represent(decoded, sizeof(data) * 8);
}







