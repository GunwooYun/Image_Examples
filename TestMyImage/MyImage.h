#pragma once
//#include <windows.h>
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <cstdio>
#include "bmpheader.h"
#define CLIP(x) (x < 0) ? 0 : x > 255 ? 255 : x;


template <typename T> // class <->typename 관계 없음
class CMyImage {
protected:
	int m_nChannels; // 채널 수
	int m_nHeight; // 세로 픽셀 수
	int m_nWidth; // 가로 픽셀 수
	int m_nWStep; // 행당 데이터 원소 수
	T* m_pImageData; // 픽셀 배열 포인터

public:
	/* 생성자 선언 및 구현 */
	CMyImage(void)
		: m_nChannels(0)
		, m_nHeight(0)
		, m_nWidth(0)
		, m_nWStep(0)
		, m_pImageData(NULL) {
	}

	CMyImage(int nWidth, int nHeight, int nChannels = 1)
		: m_nChannels(nChannels)
		, m_nHeight(nHeight)
		, m_nWidth(nWidth)
		, m_nWStep(((nWidth* nChannels * sizeof(T) + 3) & ~3) / sizeof(T)) { // 4의 배수로 만들기 위해 +3 하고 3의보수

		m_pImageData = new T[m_nHeight * m_nWStep];

	}

	/* 복사생성자 선언 및 구현 */
	CMyImage(const CMyImage& myImage) {
		m_nChannels = myImage.m_nChannels;
		m_nHeight = myImage.m_nHeight;
		m_nWidth = myImage.m_nWidth;
		m_nWStep = myImage.m_nWStep;
		m_pImageData = new T[m_nHeight * m_nWStep]; // 깊은 복사
		memcpy(m_pImageData, myImage.m_pImageData, m_nHeight * m_nWStep * sizeof(T)); // 메모리 복사
	}

	/* 연산자 함수 선언 및 구현 */
	CMyImage& operator=(const CMyImage& myImage) {
		if (this == &myImage)
			return *this;
		m_nChannels = myImage.m_nChannels;
		m_nHeight = myImage.m_nHeight;
		m_nWidth = myImage.m_nWidth;
		m_nWStep = myImage.m_nWStep;

		if (m_pImageData)
			delete[] m_pImageData;
		if (myImage.m_pImageData != NULL) {
			m_pImageData = new T[m_nHeight * m_nWStep]; // 깊은 복사
			memcpy(m_pImageData, myImage.m_pImageData, m_nHeight * m_nWStep * sizeof(T)); // 값 복사
		}
		return *this;
	}

	/* 타입이 서로 다른 객체 복사생성자 */
	template <typename From>
	CMyImage(const CMyImage<From>& myImage) {
		m_nChannels = myImage.GetChannel();
		m_nHeight = myImage.GetHeight();
		m_nWidth = myImage.GetWidth();
		m_nWStep = ((m_nWidth * m_nChannels * sizeof(T) + 3) & ~3) / sizeof(T);
		m_pImageData = new T[m_nHeight * m_nWidth];

		int nWStep = myImage.GetWStep();

		if (sizeof(T) == 1) {
			for (int r = 0; r < m_nHeight; r++) {
				T* pDst = GetPtr(r); // 각 원소의 주소값 리턴
				From* pSrc = myImage.GetPtr(r);
				for (int c = 0; c < nWStep; c++)
					pDst[c] = (T)CLIP(pSrc[c]);
			}
		}
	}

	/* 소멸자 선언 및 구현 */
	~CMyImage(void) {
		if (m_pImageData) delete[] m_pImageData;
	}

	/* 함수 선언 및 구현 */
	bool LoadImage(const char* filename) {
		assert(sizeof(T) == 1); // BYTE 형의 경우만 가능
		if (!strcmp(".bmp", &filename[strlen(filename) - 4])) { // 확장자 BMP 확인
			FILE* pFile = NULL; // 파일 구조체 선언
			pFile = fopen(filename, "rb"); // 바이너리 읽기 모드

			if (!pFile) return false; // 파일구조체가 NULL일 경우

			BITMAPFILEHEADER fileHeader;

			/* fread(읽어들인 데이터 저장 buffer, 읽을 크기, 읽을 크기의 갯수, 읽어들일 stream) */
			if (!fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pFile)) {
				fclose(pFile);
				return false;
			}

			/* 'BM' 문자 검사 */
			if (fileHeader.bfType != 0x4D42) { // 0x4D M 0x42 B
				fclose(pFile);
				return false;
			}

			BITMAPINFOHEADER infoHeader;

			if (!fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pFile)) {
				fclose(pFile);
				return false;
			}

			if (infoHeader.biBitCount != 8 && infoHeader.biBitCount != 24) {
				fclose(pFile);
				return false;
			}

			if (m_nWidth != infoHeader.biWidth && m_nHeight != infoHeader.biHeight && m_nChannels != infoHeader.biBitCount / 8) {
				if (m_pImageData) delete[] m_pImageData;
				m_nChannels = infoHeader.biBitCount / 8;
				m_nHeight = infoHeader.biHeight;
				m_nWidth = infoHeader.biWidth;
				m_nWStep = (m_nWidth * m_nChannels * sizeof(T) + 3) & ~3;

				m_pImageData = new T[m_nHeight * m_nWStep];
			}

			/* 파일 위치 포인터 위치이동 fseek( 파일 포인터, 이동할 오프셋, 초기 위치) */
			fseek(pFile, fileHeader.bfOffBits, SEEK_SET);

			int r;
			for (r = m_nHeight - 1; r >= 0; r--) {
				if (!fread(&m_pImageData[r * m_nWStep], sizeof(BYTE), m_nWStep, pFile)) {
					fclose(pFile);
					return false;
				}
			}

			fclose(pFile);
			return true;
		}

		else return false;
	}

	bool SaveImage(const char* filename) {
		assert(sizeof(T) == 1); // BYTE 형의 경우만 가능

		if (!strcmp(".bmp", &filename[strlen(filename) - 4])) { // 확장자 BMP 확인
			FILE* pFile = NULL; // 파일 구조체 선언
			pFile = fopen(filename, "wb"); // 바이너리 읽기 모드

			if (!pFile) return false; // 파일구조체가 NULL일 경우

			/* BMP 파일 정보 */
			BITMAPFILEHEADER fileHeader;

			fileHeader.bfType = 0x4D42; // 'BM'
			fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_nWStep * m_nHeight + (m_nChannels == 1) * 1024; // 파일크기 : 4byte 정수
			fileHeader.bfReserved1 = 0; // 추후 확장을 위한 필드
			fileHeader.bfReserved2 = 0; // 추후 확장을 위한 필드
			fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (m_nChannels == 1) * 256 * sizeof(RGBQUAD); // 실제 이미지까지의 오프셋 : 바이트

			fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pFile);

			/* BMP 이미지 정보 */
			BITMAPINFOHEADER infoHeader;
			infoHeader.biSize = sizeof(BITMAPINFOHEADER); // 현 구조체의 크기 : 4byte
			infoHeader.biWidth = m_nWidth; // 이미지의 폭(픽셀 단위) : 4byte
			infoHeader.biHeight = m_nHeight; // 이미지의 높이(픽셀 단위) : 4byte
			infoHeader.biPlanes = 1; // 비트 플레인 수(항상 1) : 2byte
			infoHeader.biBitCount = m_nChannels * 8; //픽셀당 비트 수 : 2byte
			infoHeader.biCompression = 0; // 압축 유형 : 4byte
			infoHeader.biSizeImage = m_nWStep * m_nHeight; // 이미지 크기(압축 전 바이트 단위) : 4byte
			infoHeader.biClrImportant = 0; // 중요한 색상 인덱스(0인 경우 전체) : 4byte
			infoHeader.biClrUsed = 0;
			infoHeader.biXPelsPerMeter = 0;
			infoHeader.biYPelsPerMeter = 0;

			fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);

			/* 팔레트 정보 */
			if (m_nChannels == 1) {
				for (int l = 0; l < 256; l++) {
					RGBQUAD GrayPalette = { (BYTE)l, (BYTE)l, (BYTE)l, (BYTE)0 }; // R,G,B 값이 모두 같으면 그레이 색 표현
					fwrite(&GrayPalette, sizeof(RGBQUAD), 1, pFile);
				}
			}

			int r;
			for (r = m_nHeight - 1; r >= 0; r--) {
				fwrite(&m_pImageData[r * m_nWStep], sizeof(BYTE), m_nWStep, pFile);
			}
			fclose(pFile);
			return true;
		}
		else {
			return false;
		}
	}

	bool IsEmpty() const {
		return m_pImageData ? false : true;
	}

	void SetConstValue(T val) {
		if (val == 0) {
			memset(m_pImageData, 0, m_nWStep * m_nHeight * sizeof(T));
			return;
		}

		if (sizeof(T) == 1) {
			memset(m_pImageData, val, m_nWStep * m_nHeight);
		}
		else {
			T* pData = m_pImageData;
			for (int r = 0; r < m_nHeight; r++) {
				for (int c = 0; c < m_nWidth; c++) {
					pData[c] = val;
				}
				pData += m_nWStep;
			}
		}
	}

	/* 반복되는 호출의 오버헤드를 줄이고자 inline 선언 */
	inline T& GetAt(int x, int y, int c = 0) const {
		assert(x >= 0 && x < m_nWidth&& y >= 0 && y < m_nHeight);
		return m_pImageData[m_nWStep * y + m_nChannels * x + c];
	}

	int GetChannel() const { return m_nChannels; }
	int GetHeight() const { return m_nHeight; }
	int GetWidth() const { return m_nWidth; }
	int GetWStep() const { return m_nWStep; } // 행당 원소수 리턴
	T* GetPtr(int r = 0, int c = 0) const { return m_pImageData + r * m_nWStep + c; }
};


typedef CMyImage <BYTE	> CByteImage;
typedef CMyImage <int	> CIntImage;
typedef CMyImage <float	> CFloatImage;
typedef CMyImage <double> CDoubleImage;
