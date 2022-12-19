#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <limits.h>                     /* USHRT_MAX 상수를 위해서 사용한다. */
#include "bmpHeader.h"

//ARM 에서 패딩시 나머지로 남는 1~2바이트를 4바이트로 교정해주는 수식
#define widthBytes(bits) (((bits)+31)/32*4)

/* 이미지 데이터의 경계 검사를 위한 매크로 */
#define LIMIT_UBYTE(n) ((n)>UCHAR_MAX)?UCHAR_MAX:((n)<0)?0:(n)

typedef unsigned char ubyte;                                // unsigned ubyte 자료형 선언
                                
int main(int argc, char** argv)                                                                                                                                     
{
    FILE* fp;                                               // File
    BITMAPFILEHEADER bmpHeader;                             /* BMP FILE INFO */
    BITMAPINFOHEADER bmpInfoHeader;                         /* BMP IMAGE INFO */
    RGBQUAD *palrgb;                                        // RGB palete
    ubyte *inimg, *outimg;                                  // 입출력 이미지 자료형 선언
    int x, y, z;                                    
    float elemSize;                                         // 헤더의 구조체 사이즈
    
    int mask = 0b0;                                         // 마스크 값 초기화

    if(argc != 3)                                           // argument count 3이 아닐 때 프린트 0 = 실행파일, 1 = 입력파일, 2 = 출력파일 
    {
        fprintf(stderr, "usage : %s input.bmp output.bmp\n", argv[0]);
        return -1;
    }
    
    /***** read bmp *****/ 
    if((fp=fopen(argv[1], "rb")) == NULL)                   // 첫 번째 입력된 파일 이름이 널값일 때 실패 메시지 
    { 
        fprintf(stderr, "Error : Failed to open file...₩n"); 
        return -1;
    }

    /* BITMAPFILEHEADER 구조체의 데이터 */
    fread(&bmpHeader, sizeof(BITMAPFILEHEADER), 1, fp);
                                                                 
    /* BITMAPINFOHEADER 구조체의 데이터 */
    fread(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

#if 0
    /* 트루 컬러를 지원하지 않으면 표시할 수 없다. */
    if(bmpInfoHeader.biBitCount != 24) {
        perror("This image file doesn't supports 24bit color\n");
        fclose(fp);
        return -1;
    }
#endif 

    // 사이즈 이미지 정의 = 비트수* 가로픽셀* 세로픽셀
    if(bmpInfoHeader.SizeImage != 0)          
    {
	    bmpInfoHeader.SizeImage = widthBytes(bmpInfoHeader.biBitCount * bmpInfoHeader.biWidth)*bmpInfoHeader.biHeight;
    }

    /* 이미지의 해상도(넓이 × 깊이) */
    printf("Resolution : %d x %d\n", bmpInfoHeader.biWidth, bmpInfoHeader.biHeight);        
    printf("Bit Count : %d\n", bmpInfoHeader.biBitCount);     /* 픽셀당 비트 수(색상) */
    printf("Size Image : %d\n", bmpInfoHeader.SizeImage );
    printf("Color : %d\n", bmpInfoHeader.biClrUsed);
	
    // 사용되는 색상 값 256 (0 ~ 255)
    // 조건 : 8비트 이미지에서 (팔레트가 있을때), ClrUsed가 0 이면 256개 컬러가 있다.
	if(bmpInfoHeader.biBitCount == 8 && bmpInfoHeader.biClrUsed == 0)
	{
		bmpInfoHeader.biClrUsed = 256;
	}

    // Heap 영역에 팔레트 영역 동적할당 
    palrgb = (RGBQUAD*)malloc(sizeof(RGBQUAD)*bmpInfoHeader.biClrUsed);
    
    // file 에서 binarydata 를 얻어 palrgb 에 저장
    fread(palrgb, sizeof(RGBQUAD), bmpInfoHeader.biClrUsed, fp); 

    // 사용된 rgb 값들을 순서대로 출력
    for(int i = 0; i < bmpInfoHeader.biClrUsed; i++) 
    {	
	    printf("%d : %x %x %x %x\n", i, palrgb[i].rgbBlue, palrgb[i].rgbGreen, palrgb[i].rgbRed, palrgb[i].rgbReserved);
    }
    
    // in, out img 를 동적할당 
    inimg = (ubyte*)malloc(sizeof(ubyte)*bmpInfoHeader.SizeImage); 
    outimg = (ubyte*)malloc(sizeof(ubyte)*(bmpInfoHeader.biWidth*bmpInfoHeader.biHeight*3));
   
    // file 에서 binarydata 를 얻어 inimg 에 저장
    fread(inimg, sizeof(ubyte), bmpInfoHeader.SizeImage, fp); 
    
    fclose(fp);

    // bitCount 에 따라 mask 값을 할당 (=1)
    for(x = 0; x < bmpInfoHeader.biBitCount; x++)
	mask |= 0b1 << x;
    
    printf:("%d", mask);
    // BitCount 24 / 8 = 3
    elemSize = bmpInfoHeader.biBitCount / 8.;
    int pos = 0; 

    for(x = 0; x < bmpInfoHeader.biWidth*bmpInfoHeader.biHeight*elemSize; x++) {      
		for(int i = 8- bmpInfoHeader.biBitCount; i >= 0; i-=bmpInfoHeader.biBitCount){
		int num = inimg[x];   
       		int res = num >> i & mask;
	        outimg[pos++]=palrgb[res].rgbBlue;
        	outimg[pos++]=palrgb[res].rgbGreen;
		outimg[pos++]=palrgb[res].rgbRed;
		}
    }         
     
    /***** write bmp *****/ 
    if((fp=fopen(argv[2], "wb"))==NULL) { 
        fprintf(stderr, "Error : Failed to open file...₩n"); 
        return -1;
    }

    bmpInfoHeader.biBitCount = 24;
    bmpInfoHeader.SizeImage = bmpInfoHeader.biWidth*bmpInfoHeader.biHeight*3;
    bmpInfoHeader.biClrUsed = 0;
    bmpHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpInfoHeader.SizeImage;
    bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    /* BITMAPFILEHEADER 구조체의 데이터 */
    fwrite(&bmpHeader, sizeof(BITMAPFILEHEADER), 1, fp);

    /* BITMAPINFOHEADER 구조체의 데이터 */
    fwrite(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    fwrite(outimg, sizeof(unsigned char), bmpInfoHeader.SizeImage, fp);

    fclose(fp); 
    
    free(inimg); 
    free(outimg);
    
    return 0;
}
