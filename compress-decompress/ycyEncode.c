#pragma pack(push)
#pragma pack(1)		//�ڴ�����Ϊ1���ֽڶ���ģʽ

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ycy.h"

typedef struct HUF_FILE_HEAD {
	unsigned char flag[3];				//ѹ���������ļ�ͷ����־ ycy
	unsigned char alphaVariety;		//�ַ�����
	unsigned char lastValidBit;		//���һ���ֽڵ���Чλ��
	unsigned char unused[11];			//���ÿռ�
} HUF_FILE_HEAD;								//����ṹ���ܹ�ռ��16���ֽڵĿռ�

typedef struct ALPHA_FREQ {
	unsigned char alpha;		//�ַ�,���ǵ��ļ����к��֣����Զ����unsigned char
	int freq;								//�ַ����ֵ�Ƶ��
} ALPHA_FREQ;

typedef struct HUFFMAN_TAB {
	ALPHA_FREQ alphaFreq;
	int leftChild;
	int rightChild;
	boolean visited;
	char *code;
} HUFFMAN_TAB;

boolean isFileExist(char *fileName);
ALPHA_FREQ *getAlphaFreq(char *sourceFileName, int *alphaVariety);
void showAlphaFreq(ALPHA_FREQ *alphaFreq, int alphaVariety);
HUFFMAN_TAB *initHuffmanTab(ALPHA_FREQ *alphaFreq, int alphaVariety, int *hufIndex);
void destoryHuffmanTab(HUFFMAN_TAB *huffmanTab, int alphaVariety);
void showHuffmanTab(HUFFMAN_TAB *huffmanTab, int count);
int getMinFreq(HUFFMAN_TAB *huffmanTab, int count);
void creatHuffmanTree(HUFFMAN_TAB *huffmanTab, int alphaVariety);
void makeHuffmanCode(HUFFMAN_TAB *huffmanTab, int root, int index, char *code);
void huffmanEncoding(HUFFMAN_TAB *huffmanTab, char *sourceFileName, char *targetFileName,
										 int *hufIndex, int alphaVariety, ALPHA_FREQ *alphaFreq);
int getlastValidBit(HUFFMAN_TAB *huffmanTab, int alphaVariety);

//ȡ���һ���ֽڵ���Чλ��
int getlastValidBit(HUFFMAN_TAB *huffmanTab, int alphaVariety) {
	int sum = 0;
	int i;
	
	for(i = 0; i < alphaVariety; i++) {
		sum += strlen(huffmanTab[i].code) * huffmanTab[i].alphaFreq.freq;
		//�����ִ����һ���������ݳ��ȳ���int�ı�ʾ��Χ���ͻ����
		sum &= 0xFF; //0xFF��Ϊ������λ1111 1111��������sumʼ�������һ���ֽ�,8λ
	}
	//���������������7λ�����ƣ�����Ϊһ���ֽڣ���ô����һ���ֽ�ֻ��7λΪ��Чλ�����඼������λ��
	//����ֻ��Ҫȡ������ֽڵ���7����Чλ������sum��8ȡ�༴��
	//sum = sum % 8 <=> sum = sum & 0x7
	//�������һ���ֽڵ���Чλ��
	sum &= 0x7;
		
	return sum == 0 ? 8 : sum;
}

//hufIndex�±�Ϊ�ַ���ASCII�룬��ֵΪ���ַ��ڹ��������е��±꣬�γɼ�ֵ�ԡ��ڰ��ַ���ת��Ϊ����������ʱ����ʵ�ֿ��ٲ���
void huffmanEncoding(HUFFMAN_TAB *huffmanTab, char *sourceFileName, char *targetFileName, int *hufIndex,
										 int alphaVariety, ALPHA_FREQ *alphaFreq) {
	FILE *fpIn;
	FILE *fpOut;
	int ch;
	unsigned char value;
	int bitIndex = 0;
	int i;
	char *hufCode = NULL;
	HUF_FILE_HEAD fileHead = {'y', 'c', 'y'};
	
	fpIn = fopen(sourceFileName, "rb");
	fpOut = fopen(targetFileName, "wb");
	
	fileHead.alphaVariety = (unsigned char) alphaVariety;
	fileHead.lastValidBit = getlastValidBit(huffmanTab, alphaVariety);
	 
//	size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
//	ptr -- ����ָ��Ҫ��д���Ԫ�������ָ�롣
//	size -- ����Ҫ��д���ÿ��Ԫ�صĴ�С�����ֽ�Ϊ��λ��
//	nmemb -- ����Ԫ�صĸ�����ÿ��Ԫ�صĴ�СΪ size �ֽڡ�
//	stream -- ����ָ�� FILE �����ָ�룬�� FILE ����ָ����һ���������
	//���ļ�ͷ��д��Ԫ����
	fwrite(&fileHead, sizeof(HUF_FILE_HEAD), 1, fpOut);
	//��Ԫ���ݺ�д�ַ������Ƶ�ȣ���ѹ��ʱ��Ҫ����Щ����һģһ���µĹ�������
	fwrite(alphaFreq, sizeof(ALPHA_FREQ), alphaVariety, fpOut);
	
	ch = fgetc(fpIn);
	while(!feof(fpIn)) {
		hufCode = huffmanTab[hufIndex[ch]].code;
		//��ÿ���ַ��Ĺ���������һ��һ������
		//������ַ�'0'����ת��Ϊ�����Ƶ�0
		//������ַ�'1'����ת��Ϊ�����Ƶ�1
		for(i = 0; hufCode[i]; i++) {
			if('0' == hufCode[i]) {
				//valueΪһ���ֽ�
				//�ӵ�1λ���θ�ֵ�������ڰ�λ��һ���ֽڣ��ˣ���д���ļ���
				CLR_BYTE(value, bitIndex);
			} else {
				SET_BYTE(value, bitIndex);
			}
			bitIndex++;
			if(bitIndex >= 8) {
				bitIndex = 0;
				fwrite(&value, sizeof(unsigned char), 1, fpOut);
			}
		}
		ch = fgetc(fpIn);
	}
	//������һ�β���һ���ֽڣ���Ȼ��Ҫд���ļ��У�ע�⣺д������һ���ֽڿ��ܻ��������λ
	if(bitIndex) {
		fwrite(&value, sizeof(unsigned char), 1, fpOut);
	}
	
	fclose(fpIn);
	fclose(fpOut);
}

void makeHuffmanCode(HUFFMAN_TAB *huffmanTab, int root, int index, char *code) {
	if(huffmanTab[root].leftChild != -1 && huffmanTab[root].rightChild != -1) {
		code[index] = '1';
		makeHuffmanCode(huffmanTab, huffmanTab[root].leftChild, index + 1, code);
		code[index] = '0';
		makeHuffmanCode(huffmanTab, huffmanTab[root].rightChild, index + 1, code);
	} else {
		code[index] = 0;
		strcpy(huffmanTab[root].code, code);
	}
}

void creatHuffmanTree(HUFFMAN_TAB *huffmanTab, int alphaVariety) {
	int i;
	int leftChild;
	int rightChild;

	//huffmanTabʹ��ʣ�µ� alphaVariety - 1���ռ�
	for(i = 0; i < alphaVariety - 1; i++) {
		leftChild = getMinFreq(huffmanTab, alphaVariety + i);
		rightChild = getMinFreq(huffmanTab, alphaVariety + i);
		huffmanTab[alphaVariety + i].alphaFreq.alpha = '#';
		huffmanTab[alphaVariety + i].alphaFreq.freq = huffmanTab[leftChild].alphaFreq.freq
																									 + huffmanTab[rightChild].alphaFreq.freq;
		huffmanTab[alphaVariety + i].leftChild = leftChild;
		huffmanTab[alphaVariety + i].rightChild = rightChild;
		huffmanTab[alphaVariety + i].visited = FALSE;
	}
}

//�ڹ�����������û�з��ʹ�����СƵ���±�
int getMinFreq(HUFFMAN_TAB *huffmanTab, int count) {
	int index;
	int minIndex = NOT_INIT;

	for(index = 0; index < count; index++) {
		if(FALSE == huffmanTab[index].visited) {
			if(NOT_INIT == minIndex || huffmanTab[index].alphaFreq.freq < huffmanTab[minIndex].alphaFreq.freq) {
				minIndex = index;
			}
		}
	}
	huffmanTab[minIndex].visited = TRUE;

	return minIndex;
}

void showHuffmanTab(HUFFMAN_TAB *huffmanTab, int count) {
	int i;

	printf("%-4s %-4s %-4s %-6s %-6s %-6s %s\n", "�±�", "�ַ�", "Ƶ��", "����", "�Һ���", "visited", "code");
	for (i = 0; i < count; i++) {
		printf("%-5d %-4c %-5d %-6d %-7d %-4d %s\n",
				i,
				huffmanTab[i].alphaFreq.alpha,
				huffmanTab[i].alphaFreq.freq,
				huffmanTab[i].leftChild,
				huffmanTab[i].rightChild,
				huffmanTab[i].visited,
				(huffmanTab[i].code ? huffmanTab[i].code : "��"));
	}
}

void destoryHuffmanTab(HUFFMAN_TAB *huffmanTab, int alphaVariety) {
	int i;
	
	for(i = 0; i < alphaVariety; i++) {
		free(huffmanTab[i].code);
	}
	
	free(huffmanTab);
}

HUFFMAN_TAB *initHuffmanTab(ALPHA_FREQ *alphaFreq, int alphaVariety, int *hufIndex) {
	int i;
	HUFFMAN_TAB *huffmanTab = NULL;

	huffmanTab = (HUFFMAN_TAB *) calloc(sizeof(HUFFMAN_TAB), 2 * alphaVariety - 1);
	//huffmanTab������ 2 * alphaVariety - 1��С�Ŀռ䣬����ֻ���� alphaVariety������ʣalphaVariety - 1��
	for(i = 0; i < alphaVariety; i++) {
		hufIndex[alphaFreq[i].alpha] = i;	//�ѹ��������е��ַ������Ӧ���±��γɼ�ֵ��,�浽hufIndex��
		huffmanTab[i].alphaFreq = alphaFreq[i];
		huffmanTab[i].leftChild = huffmanTab[i].rightChild = -1;
		huffmanTab[i].visited = FALSE;
		huffmanTab[i].code = (char *) calloc(sizeof(char), alphaVariety);
	}

	return huffmanTab;
}

void showAlphaFreq(ALPHA_FREQ *alphaFreq, int alphaVariety) {
	int i;

	for(i = 0; i < alphaVariety; i++) {
		int ch = alphaFreq[i].alpha;
		//�ַ�������ASCII���ʾ��Χ
		if(ch > 127) {
			printf("[%d]: %d\n", ch, alphaFreq[i].freq);
		} else {
			printf("[%c]: %d\n", ch, alphaFreq[i].freq);
		}
	}
}

//��sourceFileName�ļ��ж�ȡ�ַ���
//feof()�������ǡ��ļ�������־������������һ����������־������
//��ʾ������һ�ε��ļ���д�����Ƿ�ɹ���
//�ոմ��ļ���feof()�����ĳ�ʼֵΪ0; ����һ���ļ�����ʧ��ʱ�����ط�0
ALPHA_FREQ *getAlphaFreq(char *sourceFileName, int *alphaVariety) {
	int freq[256] = {0};
	int i;
	int index;
	ALPHA_FREQ *alphaFreq = NULL;
	FILE *fpIn;
	int ch;
	
	fpIn = fopen(sourceFileName, "rb");
	
	/*ͳ�������ַ���Ƶ��*/
	ch = fgetc(fpIn);
	while(!feof(fpIn)) {
		freq[ch]++;
		ch = fgetc(fpIn);
	}
	fclose(fpIn);

	/*ͳ�������ַ�������*/
	for(i = 0; i < 256; i++) {
		if(freq[i]) {
			(*alphaVariety)++;
		}
	}

	alphaFreq = (ALPHA_FREQ *) calloc(sizeof(ALPHA_FREQ), *alphaVariety);
	for(i = index = 0; i < 256; i++) {
		if(freq[i]) {
			alphaFreq[index].alpha = i;
			alphaFreq[index].freq = freq[i];
			index++;
		}
	}

	return alphaFreq;
}

boolean isFileExist(char *fileName) {
	FILE *fp;
	
	fp = fopen(fileName, "rb");
	if (NULL == fp) {
		return FALSE;
	}
	
	fclose(fp);
	return TRUE;
}

int main(int argc, char const *argv[]) {
	char sourceFileName[256] = {0};
	char targetFileName[256] = {0};
	ALPHA_FREQ *alphaFreq = NULL;				//ͳ���ַ���Ƶ�ȵ�����
	int alphaVariety = 0;								// �ַ�����
	HUFFMAN_TAB *huffmanTab = NULL;			//��������
	char *code = NULL;									//�洢�ַ��Ĺ���������
	int hufIndex[256] = {0};						//�±�Ϊ�ַ���ASCII�룬��ֵΪ���ַ��ڹ��������е��±꣬�γɼ�ֵ��
		
	// if(argc != 3) {
	// 	printf("intput command correctly: ycyEnCode <filename> <destFilename>\n");
	// 	return 0;
	// }

	snprintf(sourceFileName, sizeof(sourceFileName), "%s", "lz.bmp") ;
	
	//�ڶ�������ΪԴ�ļ���
	// strcpy(sourceFileName, argv[1]);
	if(!isFileExist(sourceFileName)) {
		printf("Դ�ļ�(%s)�����ڣ�\n", sourceFileName);
		return 0;
	}
	
	// strcpy(targetFileName, argv[2]);
	snprintf(targetFileName, sizeof(targetFileName), "%s", "lztmp.bmp") ;

	
	alphaFreq = getAlphaFreq(sourceFileName, &alphaVariety);
	//showAlphaFreq(alphaFreq, alphaVariety);
	
	huffmanTab = initHuffmanTab(alphaFreq, alphaVariety, hufIndex);
	creatHuffmanTree(huffmanTab, alphaVariety);
	code = (char *) calloc(sizeof(char), alphaVariety);
	makeHuffmanCode(huffmanTab, 2 * alphaVariety - 2, 0, code);
	//showHuffmanTab(huffmanTab, 2 * alphaVariety - 1);

	huffmanEncoding(huffmanTab, sourceFileName, targetFileName, hufIndex, alphaVariety, alphaFreq);
	
	destoryHuffmanTab(huffmanTab, alphaVariety);
	free(alphaFreq);
	free(code);

	system("pause");
	
	return 0;
}

#pragma pack(pop)