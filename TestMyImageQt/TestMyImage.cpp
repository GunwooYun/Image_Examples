#include <QApplication>
#include <QLabel>
#include <QPixmap>
//#include <iostream>
#include "MyImage.h"
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	CByteImage image1(640, 480); // 생성자 호출
	image1.SetConstValue(0);	 // 0으로 초기화

	CByteImage image2(image1); // 복사 생성자 호출
	CByteImage image3;		   // 기본 생성자 호출
	image3 = image1;		   // 대입 연산자 호출

	int nWidth = image1.GetWidth();   // 영상의 너비
	int nHeight = image1.GetHeight();  // 영상의 높이
	//int nChannel = image1.GetChannel(); // 영상의 채널 수

	double incX = 255.0 / nWidth;
	double incY = 255.0 / nHeight;
	int r, c;
	for (r = 0; r < nHeight; r++) // 행 단위 이동
	{
		for (c = 0; c < nWidth; c++) // 열 단위 이동
		{
			image2.GetAt(c, r) = (BYTE)(c * incX); // 가로 그라데이션
			image3.GetAt(c, r) = (BYTE)(r * incY); // 세로 그라데이션
		}
	}

	// 결과 영상 저장
	image1.SaveImage("Black.bmp");
	image2.SaveImage("GradationX.bmp");
	image3.SaveImage("GradationY.bmp");

   	QLabel* lb1 = new QLabel("Black.bmp", 0);
    	lb1->setPixmap(QPixmap("Black.bmp"));
    	lb1->show();

   	QLabel* lb2 = new QLabel("GradationX.bmp", 0);
    	lb2->setPixmap(QPixmap("GradationX.bmp"));
    	lb2->show();

   	QLabel* lb3 = new QLabel("GradationY.bmp", 0);
    	lb3->setPixmap(QPixmap("GradationY.bmp"));
    	lb3->show();

	return app.exec();
}
