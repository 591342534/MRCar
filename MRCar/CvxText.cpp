#include "stdafx.h"
#include <wchar.h>
#include <assert.h>
#include <locale.h>
#include <ctype.h>

#include "CvxText.h"
#if _DEBUG
#pragma comment(lib,"freetype251d.lib")
#else
#pragma comment(lib,"freetype251.lib")
#endif
//====================================================================
//====================================================================

// ���ֿ�

CvxText::CvxText(const char *freeType)
{
	assert(freeType != NULL);

	// ���ֿ��ļ�, ����һ������

	if(FT_Init_FreeType(&m_library)) throw;
	if(FT_New_Face(m_library, freeType, 0, &m_face)) throw;

	// ���������������

	restoreFont();

	// ����C���Ե��ַ�������

	setlocale(LC_ALL, "");
}

// �ͷ�FreeType��Դ

CvxText::~CvxText()
{
	FT_Done_Face    (m_face);
	FT_Done_FreeType(m_library);
}

// �����������:
//
// font         - ��������, Ŀǰ��֧��
// size         - �����С/�հױ���/�������/��ת�Ƕ�
// underline   - �»���
// diaphaneity   - ͸����

void CvxText::getFont(int *type, CvScalar *size, bool *underline, float *diaphaneity)
{
	if(type) *type = m_fontType;
	if(size) *size = m_fontSize;
	if(underline) *underline = m_fontUnderline;
	if(diaphaneity) *diaphaneity = m_fontDiaphaneity;
}

void CvxText::setFont(int *type, CvScalar *size, bool *underline, float *diaphaneity)
{
	// �����Ϸ��Լ��

	if(type)
	{
		if(type >= 0) m_fontType = *type;
	}
	if(size)
	{
		m_fontSize.val[0] = fabs(size->val[0]);
		m_fontSize.val[1] = fabs(size->val[1]);
		m_fontSize.val[2] = fabs(size->val[2]);
		m_fontSize.val[3] = fabs(size->val[3]);
	}
	if(underline)
	{
		m_fontUnderline   = *underline;
	}
	if(diaphaneity)
	{
		m_fontDiaphaneity = *diaphaneity;
	}

	FT_Set_Pixel_Sizes(m_face, (int)m_fontSize.val[0], 0);
}

// �ָ�ԭʼ����������

void CvxText::restoreFont()
{
	m_fontType = 0;            // ��������(��֧��)

	m_fontSize.val[0] = 20;      // �����С
	m_fontSize.val[1] = 0.5;   // �հ��ַ���С����
	m_fontSize.val[2] = 0.1;   // �����С����
	m_fontSize.val[3] = 0;      // ��ת�Ƕ�(��֧��)

	m_fontUnderline   = false;   // �»���(��֧��)

	m_fontDiaphaneity = 1.0;   // ɫ�ʱ���(�ɲ���͸��Ч��)

	// �����ַ���С

	FT_Set_Pixel_Sizes(m_face, (int)m_fontSize.val[0], 0);
}

// �������(��ɫĬ��Ϊ��ɫ)

int CvxText::putText(IplImage *img, const char    *text, CvPoint pos)
{
	return putText(img, text, pos, CV_RGB(255,255,255));
}
int CvxText::putText(IplImage *img, const wchar_t *text, CvPoint pos)
{
	return putText(img, text, pos, CV_RGB(255,255,255));
}

//

int CvxText::putText(IplImage *img, const char    *text, CvPoint pos, CvScalar color)
{
	if(img == NULL) return -1;
	if(text == NULL) return -1;

	//

	int i;
	for(i = 0; text[i] != '\0'; ++i)
	{
		wchar_t wc = text[i];

		// ����˫�ֽڷ���

		if(!isascii(wc)) mbtowc(&wc, &text[i++], 2);

		// �����ǰ���ַ�

		putWChar(img, wc, pos, color);
	}
	return i;
}
int CvxText::putText(IplImage *img, const wchar_t *text, CvPoint pos, CvScalar color)
{
	if(img == NULL) return -1;
	if(text == NULL) return -1;

	//

	int i;
	for(i = 0; text[i] != '\0'; ++i)
	{
		// �����ǰ���ַ�

		putWChar(img, text[i], pos, color);
	}
	return i;
}

// �����ǰ�ַ�, ����m_posλ��

void CvxText::putWChar(IplImage *img, wchar_t wc, CvPoint &pos, CvScalar color)
{
	// ����unicode��������Ķ�ֵλͼ

	FT_UInt glyph_index = FT_Get_Char_Index(m_face, wc);
	FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
	FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_MONO);

	//

	FT_GlyphSlot slot = m_face->glyph;

	// ������

	int rows = slot->bitmap.rows;
	int cols = slot->bitmap.width;

	//

	for(int i = 0; i < rows; ++i)
	{
		for(int j = 0; j < cols; ++j)
		{
			int off  = ((img->origin==0)? i: (rows-1-i))
				* slot->bitmap.pitch + j/8;

			if(slot->bitmap.buffer[off] & (0xC0 >> (j%8)))
			{
				int r = (img->origin==0)? pos.y - (rows-1-i): pos.y + i;;
				int c = pos.x + j;

				if(r >= 0 && r < img->height
					&& c >= 0 && c < img->width)
				{
					CvScalar scalar = cvGet2D(img, r, c);

					// ����ɫ���ں�

					float p = m_fontDiaphaneity;
					for(int k = 0; k < 4; ++k)
					{
						scalar.val[k] = scalar.val[k]*(1-p) + color.val[k]*p;
					}

					cvSet2D(img, r, c, scalar);
				}
			}
		} // end for
	} // end for

	// �޸���һ���ֵ����λ��

	double space = m_fontSize.val[0]*m_fontSize.val[1];
	double sep   = m_fontSize.val[0]*m_fontSize.val[2];

	pos.x += (int)((cols? cols: space) + sep);
}