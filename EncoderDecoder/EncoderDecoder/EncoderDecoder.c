#include <stdio.h>
#include "Encoder.h"

int main()
{
	int data = 3;
	encode(&data, sizeof(data) * 8);


	//�� ����� ���� ���� �����
	readProtectionData();
}

