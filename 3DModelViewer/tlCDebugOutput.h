#pragma once

#include <vector>
#include <Core/tlCSingleton.h>
#include <Core/tlMathTypes.h>
#include <imgui/imgui.h>

class CDebugOutput : public CSingleton<CDebugOutput>
{
public:

	const static int NAME_SIZE = 50;
	const static int VALUE_SIZE = 100;

	struct DEBUG_CELL {
		int iPage;
		char szName[NAME_SIZE];
		int iNameHash;
		char szValue[4][VALUE_SIZE];
		int	iRowCount;
		int iTag;
	};

	typedef std::vector<DEBUG_CELL> DebugList;

	CDebugOutput(void);
	~CDebugOutput(void);

	void Clear();
	void Set(int iPage, const char* pszName, int iValue, int* piOptContext = NULL);
	void Set(int iPage, const char* pszName, float fValue, int iPrec = 2, int* piOptContext = NULL);
	void Set(int iPage, const char* pszName, const tochal::vector2& v, int iPrec = 2, int* piOptContext = NULL);
	void Set(int iPage, const char* pszName, const tochal::vector3& v, int iPrec = 2, int* piOptContext = NULL);
	void Set(int iPage, const char* pszName, const tochal::vector4& v, int iPrec = 2, int* piOptContext = NULL);
	void Set(int iPage, const char* pszName, const tochal::matrix& m, int iPrec = 2, int* piOptContext = NULL);
	void Set(int iPage, const char* pszName, const char* pszValue, int* piOptContext = NULL);
	void Update(const CRect& rc, int iPage, bool& bVisible, ImVec2 vSizeOnFirstUse, ImGuiWindowFlags flags, float fBgAlpha);
	void Render(LPD3DXFONT pd3dxFont, CRect& rc, int iPage);
private:

	DebugList m_list;
	int GetCell(int iPage, const char* pszName, int* piNameHash = NULL);
	int Find(int iPage, const char* pszName, int* piNameHash = NULL);
};

