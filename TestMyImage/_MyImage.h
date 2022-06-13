#pragma once
#include <windows.h>
#include <assert.h>
#include <cstdio>
#define CLIP(x) (x < 0) ? 0 : x > 255 ? 255 : x;


template <typename T> // class <->typename ���� ����
class CMyImage {
protected:
	int m_nChannels; // ä�� ��
	int m_nHeight; // ���� �ȼ� ��
	int m_nWidth; // ���� �ȼ� ��
	int m_nWStep; // ��� ������ ���� ��
	T* m_pImageData; // �ȼ� �迭 ������

public:
	/* ������ ���� �� ���� */
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
		, m_nWStep(((nWidth* nChannels * sizeof(T) + 3) & ~3) / sizeof(T)) { // 4�� ����� ����� ���� +3 �ϰ� 3�Ǻ���

		m_pImageData = new T[m_nHeight * m_nWStep];

	}

	/* ��������� ���� �� ���� */
	CMyImage(const CMyImage& myImage) {
		m_nChannels = myImage.m_nChannels;
		m_nHeight = myImage.m_nHeight;
		m_nWidth = myImage.m_nWidth;
		m_nWStep = myImage.m_nWStep;
		m_pImageData = new T[m_nHeight * m_nWStep]; // ���� ����
		memcpy(m_pImageData, myImage.m_pImageData, m_nHeight * m_nWStep * sizeof(T)); // �޸� ����
	}

	/* ������ �Լ� ���� �� ���� */
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
			m_pImageData = new T[m_nHeight * m_nWStep]; // ���� ����
			memcpy(m_pImageData, myImage.m_pImageData, m_nHeight * m_nWStep * sizeof(T)); // �� ����
		}
		return *this;
	}

	/* Ÿ���� ���� �ٸ� ��ü ��������� */
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
				T* pDst = GetPtr(r); // �� ������ �ּҰ� ����
				From* pSrc = myImage.GetPtr(r);
				for (int c = 0; c < nWStep; c++)
					pDst[c] = (T)CLIP(pSrc[c]);
			}
		}
	}

	/* �Ҹ��� ���� �� ���� */
	~CMyImage(void) {
		if (m_pImageData) delete[] m_pImageData;
	}

	/* �Լ� ���� �� ���� */
	bool LoadImage(const char* filename) {
		assert(sizeof(T) == 1); // BYTE ���� ��츸 ����
		if (!strcmp(".bmp", &filename[strlen(filename) - 4])) { // Ȯ���� BMP Ȯ��
			FILE* pFile = NULL; // ���� ����ü ����
			fopen_s(&pFile, filename, "rb"); // ���̳ʸ� �б� ���

			if (!pFile) return false; // ���ϱ���ü�� NULL�� ���

			BITMAPFILEHEADER fileHeader;

			/* fread(�о���� ������ ���� buffer, ���� ũ��, ���� ũ���� ����, �о���� stream) */
			if (!fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pFile)) {
				fclose(pFile);
				return false;
			}

			/* 'BM' ���� �˻� */
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

			/* ���� ��ġ ������ ��ġ�̵� fseek( ���� ������, �̵��� ������, �ʱ� ��ġ) */
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
		assert(sizeof(T) == 1); // BYTE ���� ��츸 ����

		if (!strcmp(".bmp", &filename[strlen(filename) - 4])) { // Ȯ���� BMP Ȯ��
			FILE* pFile = NULL; // ���� ����ü ����
			fopen_s(&pFile, filename, "wb"); // ���̳ʸ� �б� ���

			if (!pFile) return false; // ���ϱ���ü�� NULL�� ���

			/* BMP ���� ���� */
			BITMAPFILEHEADER fileHeader;

			fileHeader.bfType = 0x4D42; // 'BM'
			fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_nWStep * m_nHeight + (m_nChannels == 1) * 1024; // ����ũ�� : 4byte ����
			fileHeader.bfReserved1 = 0; // ���� Ȯ���� ���� �ʵ�
			fileHeader.bfReserved2 = 0; // ���� Ȯ���� ���� �ʵ�
			fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (m_nChannels == 1) * 256 * sizeof(RGBQUAD); // ���� �̹��������� ������ : ����Ʈ

			fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pFile);

			/* BMP �̹��� ���� */
			BITMAPINFOHEADER infoHeader;
			infoHeader.biSize = sizeof(BITMAPINFOHEADER); // �� ����ü�� ũ�� : 4byte
			infoHeader.biWidth = m_nWidth; // �̹����� ��(�ȼ� ����) : 4byte
			infoHeader.biHeight = m_nHeight; // �̹����� ����(�ȼ� ����) : 4byte
			infoHeader.biPlanes = 1; // ��Ʈ �÷��� ��(�׻� 1) : 2byte
			infoHeader.biBitCount = m_nChannels * 8; //�ȼ��� ��Ʈ �� : 2byte
			infoHeader.biCompression = BI_RGB; // ���� ���� : 4byte
			infoHeader.biSizeImage = m_nWStep * m_nHeight; // �̹��� ũ��(���� �� ����Ʈ ����) : 4byte
			infoHeader.biClrImportant = 0; // �߿��� ���� �ε���(0�� ��� ��ü) : 4byte
			infoHeader.biClrUsed = 0;
			infoHeader.biXPelsPerMeter = 0;
			infoHeader.biYPelsPerMeter = 0;

			fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);

			/* �ȷ�Ʈ ���� */
			if (m_nChannels == 1) {
				for (int l = 0; l < 256; l++) {
					RGBQUAD GrayPalette = { (BYTE)l, (BYTE)l, (BYTE)l, (BYTE)0 }; // R,G,B ���� ��� ������ �׷��� �� ǥ��
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

	/* �ݺ��Ǵ� ȣ���� ������带 ���̰��� inline ���� */
	inline T& GetAt(int x, int y, int c = 0) const {
		assert(x >= 0 && x < m_nWidth&& y >= 0 && y < m_nHeight);
		return m_pImageData[m_nWStep * y + m_nChannels * x + c];
	}

	int GetChannel() const { return m_nChannels; }
	int GetHeight() const { return m_nHeight; }
	int GetWidth() const { return m_nWidth; }
	int GetWStep() const { return m_nWStep; } // ��� ���Ҽ� ����
	T* GetPtr(int r = 0, int c = 0) const { return m_pImageData + r * m_nWStep + c; }
};


typedef CMyImage <BYTE	> CByteImage;
typedef CMyImage <int	> CIntImage;
typedef CMyImage <float	> CFloatImage;
typedef CMyImage <double> CDoubleImage;