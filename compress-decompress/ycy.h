#ifndef _YCY_H_
#define _YCY_H_

//�õ�indexλ��ֵ����indexλΪ0����GET_BYTEֵΪ�٣�����Ϊ��
#define GET_BYTE(vbyte, index) (((vbyte) & (1 << ((index) ^ 7))) != 0)
//indexλ��1
#define SET_BYTE(vbyte, index) ((vbyte) |= (1 << ((index) ^ 7)))
//indexλ��0
#define CLR_BYTE(vbyte, index) ((vbyte) &= (~(1 << ((index) ^ 7))))

typedef unsigned char boolean;

#define TRUE				1
#define FALSE				0
#define NOT_INIT		-1

#endif
