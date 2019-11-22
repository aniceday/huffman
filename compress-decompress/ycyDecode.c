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
ALPHA_FREQ *getAlphaFreq(char *sourceFileName, int *alphaVariety, HUF_FILE_HEAD fileHead);
void showAlphaFreq(ALPHA_FREQ *alphaFreq, int alphaVariety);
HUFFMAN_TAB *initHuffmanTab(ALPHA_FREQ *alphaFreq, int alphaVariety, int *hufIndex);
void destoryHuffmanTab(HUFFMAN_TAB *huffmanTab, int alphaVariety);
void showHuffmanTab(HUFFMAN_TAB *huffmanTab, int count);
int getMinFreq(HUFFMAN_TAB *huffmanTab, int count);
void creatHuffmanTree(HUFFMAN_TAB *huffmanTab, int alphaVariety);
void makeHuffmanCode(HUFFMAN_TAB *huffmanTab, int root, int index, char *code);
HUF_FILE_HEAD readFileHead(char *sourceFileName);
void huffmanDecoding(HUFFMAN_TAB *huffmanTab, char *sourceFileName, char *targetFileName, int alphaVariety, HUF_FILE_HEAD fileHead);

void huffmanDecoding(HUFFMAN_TAB *huffmanTab, char *sourceFileName, char *targetFileName, int alphaVariety, HUF_FILE_HEAD fileHead) {
	int root = 2 * alphaVariety - 2;
	FILE *fpIn;
	FILE *fpOut;
	boolean finished = FALSE;
	unsigned char value;
	unsigned char outValue;
	int index = 0;
	long fileSize;
	long curLocation;

	fpIn = fopen(sourceFileName, "rb");
	fpOut = fopen(targetFileName, "wb");
	fseek(fpIn, 0L, SEEK_END);
	fileSize = ftell(fpIn);	//�ļ��ܳ���fileSize
	fseek(fpIn, 16 + 5 * fileHead.alphaVariety, SEEK_SET);	//�Թ�ǰ��16���ֽڵ�Ԫ���ݣ�5�ֽڵ��ַ������Ƶ��
	curLocation = ftell(fpIn);
	
	//�Ӹ�������'1'���������ߣ�'0'���������ߣ�������Ҷ�ӽ�㣬���Ҷ�ӽ���±��Ӧ���ַ����ٻص�����������
	fread(&value, sizeof(unsigned char), 1, fpIn);
	while(!finished) {
		if(huffmanTab[root].leftChild == -1 && huffmanTab[root].rightChild == -1) {
			outValue = huffmanTab[root].alphaFreq.alpha;
			fwrite(&outValue, sizeof(unsigned char), 1, fpOut);
			if(curLocation >= fileSize && index >= fileHead.lastValidBit) {
				break;
			} 
			root = 2 * alphaVariety - 2;
		}
		
		//ȡ����һ���ֽڴӵ�һλ��ʼ����'1'���������ߣ�'0'����������
		//������һ���ֽڣ�8λ������Ҫ��ȡ��һ���ֽ�
		if(GET_BYTE(value, index)) {
			root = huffmanTab[root].leftChild;
		} else {
			root = huffmanTab[root].rightChild;
		}
		if(++index >= 8) {
			index = 0;
			fread(&value, sizeof(unsigned char), 1, fpIn);
			curLocation = ftell(fpIn);
		}
	}

	fclose(fpIn);
	fclose(fpOut);
}

HUF_FILE_HEAD readFileHead(char *sourceFileName) {
	HUF_FILE_HEAD fileHead;
	FILE *fp;

	fp = fopen(sourceFileName, "rb");
	//��ȡѹ�����ļ���ͷ��Ԫ���ݣ�16���ֽ�
	fread(&fileHead, sizeof(HUF_FILE_HEAD), 1, fp);
	fclose(fp);

	return fileHead;
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
		huffmanTab[alphaVariety + i].alphaFreq.freq = huffmanTab[leftChild].alphaFreq.freq + huffmanTab[rightChild].alphaFreq.freq;
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

ALPHA_FREQ *getAlphaFreq(char *sourceFileName, int *alphaVariety, HUF_FILE_HEAD fileHead) {
	int freq[256] = {0};
	int i;
	int index;
	ALPHA_FREQ *alphaFreq = NULL;
	FILE *fpIn;
	int ch;

	*alphaVariety = fileHead.alphaVariety;
	alphaFreq = (ALPHA_FREQ *) calloc(sizeof(ALPHA_FREQ), *alphaVariety);
	fpIn = fopen(sourceFileName, "rb");
	//�Թ�ǰ16���ֽڵ�Ԫ����
	fseek(fpIn, 16, SEEK_SET);
	fread(alphaFreq, sizeof(ALPHA_FREQ), *alphaVariety, fpIn);
	fclose(fpIn);

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
	HUF_FILE_HEAD fileHead;

	if(argc != 3) {
		printf("��ȷ�����ʽ: ycyDeCode <Դ�ļ���> <Ŀ���ļ���>\n");
		return 0;
	}

	//�ڶ�������ΪԴ�ļ���
	strcpy(sourceFileName, argv[1]);
	if(!isFileExist(sourceFileName)) {
		printf("Դ�ļ�(%s)�����ڣ�\n", sourceFileName);
		return 0;
	}
	fileHead = readFileHead(sourceFileName);
	if(!(fileHead.flag[0] == 'y' && fileHead.flag[1] == 'c' && fileHead.flag[2] == 'y')) {
		printf("����ʶ����ļ���ʽ\n");
	}
	//����������ΪĿ���ļ���
	strcpy(targetFileName, argv[2]);

	alphaFreq = getAlphaFreq(sourceFileName, &alphaVariety, fileHead);
	//showAlphaFreq(alphaFreq, alphaVariety);

	huffmanTab = initHuffmanTab(alphaFreq, alphaVariety, hufIndex);
	creatHuffmanTree(huffmanTab, alphaVariety);
	code = (char *) calloc(sizeof(char), alphaVariety);
	makeHuffmanCode(huffmanTab, 2 * alphaVariety - 2, 0, code);
	//showHuffmanTab(huffmanTab, 2 * alphaVariety - 1);

	huffmanDecoding(huffmanTab, sourceFileName, targetFileName, alphaVariety, fileHead);

	destoryHuffmanTab(huffmanTab, alphaVariety);
	free(alphaFreq);
	free(code);

	return 0;
}

#pragma pack(pop)