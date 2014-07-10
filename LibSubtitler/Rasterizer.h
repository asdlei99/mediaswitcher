#ifndef f_RASTERIZER_H
#define f_RASTERIZER_H

#include <vector>

#include <windows.h>

class Rasterizer {
private:
	BYTE *mpPathTypes;
	POINT *mpPathPoints;
	int mPathPoints;

	int mWidth, mHeight;

	typedef std::pair<unsigned int, unsigned int> tSpan;
	typedef std::vector<tSpan> tSpanBuffer;

	tSpanBuffer mOutline;
	tSpanBuffer mWideOutline;
	int mWideBorder;

	struct Edge {
		int next;
		int posandflag;
	} *mpEdgeBuffer;
	unsigned mEdgeHeapSize;
	unsigned mEdgeNext;

	unsigned int *mpScanBuffer;

	typedef unsigned char byte;

	byte *mpOverlayBuffer;
	int mOverlayWidth, mOverlayHeight;
	int mPathOffsetX, mPathOffsetY;
	int mOffsetX, mOffsetY;

	void _TrashPath();
	void _TrashOverlay();
	void _ReallocEdgeBuffer(int edges);
	void _EvaluateBezier(int ptbase);
	void _EvaluateLine(int pt1idx, int pt2idx);
	void _EvaluateLine(int x0, int y0, int x1, int y1);
	static void _OverlapRegion(tSpanBuffer& dst, tSpanBuffer& src, int dx, int dy);

public:
	Rasterizer();
	~Rasterizer();

	bool BeginPath(HDC hdc);
	void RenderPath(HDC hdc, int x, int y, const char *pszText);
	bool EndPath(HDC hdc);
	bool PartialBeginPath(HDC hdc, bool bClearPath);
	bool PartialEndPath(HDC hdc, long dx, long dy);
	bool ScanConvert();
	bool CreateWidenedRegion(int border);
	void DeleteOutlines();
	bool Rasterize(int xsub, int ysub, bool bBorder);
	void Draw(unsigned long *dst, int x, int y, int w, int h, int pitch, unsigned long seccolor, bool bBorder, const long *switchpts);
	void DrawShadow(unsigned long *dst, int x, int y, int w, int h, int pitch, unsigned long seccolor);
};

#endif
