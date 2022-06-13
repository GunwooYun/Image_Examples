#include "imageSrc/MyImage.h"
#include <iostream>
using namespace std;
int main() {
	CByteImage image1(640, 480); // ������ ȣ��
	image1.SetConstValue(255); // 0���� �ʱ�ȭ

	CByteImage image2(image1); // ���� ������ ȣ��
	CByteImage image3, image4;
	image3 = image1;
	image4 = image1;

	int nWidth = image1.GetWidth();
	int nHeight = image1.GetHeight();
	int nChannel = image1.GetChannel();

	/* �����ϸ� */
	double incX = 255.0 / nWidth;
	double incY = 255.0 / nHeight;
	
	int r, c, k=0;

	//printf("width : %d, height : %d\n", nWidth, nHeight);
	for (r = 0; r < nHeight; r++) {
		for (c = 0; c < nWidth; c++) {
			image2.GetAt(c, r) = (BYTE)(c * incX);
			image3.GetAt(c, r) = (BYTE)(r * incX);
			image4.GetAt(c, r) = ((BYTE)(c * incX)+(BYTE)(r * incX))/2;
			//image4.GetAt(c, r) = (BYTE)(r * incX);
			//image4.GetAt(c, r) = (BYTE)(r * incY);
			//image4.GetAt(c, r) = (BYTE)(c * incX);
			//image4.GetAt(c, r) = (BYTE)(r * incX);
		}		
	}

	image1.SaveImage("Black.bmp");
	image2.SaveImage("GradationX.bmp");
	image3.SaveImage("GradationY.bmp");
	image4.SaveImage("GradationXY.bmp");
	return 0;
}