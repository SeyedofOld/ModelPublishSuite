#include "StdAfx.h"
#include "tlCDebugOutput.h"
#include "tlBaseTypes.h"
#include <imgui/imgui.h>

using namespace tochal ;

CDebugOutput::CDebugOutput(void)
{
}


CDebugOutput::~CDebugOutput(void)
{
}

void CDebugOutput::Clear()
{
	m_list.clear();
}

int CDebugOutput::Find( int iPage, const char* pszName, int* piNameHash )
{
	bool bFound = false;
	
	if (piNameHash && ( (*piNameHash) == 0) ) {
		std::hash<std::string> hash;
		*piNameHash = hash(pszName);
	}

	int iIndex = -1;
	int i = 0;
	for ( DebugList::iterator it = m_list.begin() ; it != m_list.end() ; it++, i++ ) {

		if (piNameHash) {
			if (((*it).iPage == iPage) && ((*it).iNameHash == (*piNameHash))) {
				iIndex = i;
				bFound = true;
				break;
			}
		}
		else {
			if ( ((*it).iPage == iPage) && (0 == strcmp((*it).szName, pszName)) ) {
				iIndex = i;
				bFound = true;
				break;
			}

		}
	}

	return iIndex;
}

int CDebugOutput::GetCell( int iPage, const char* pszName, int* piNameHash )
{
	int iIndex = Find( iPage, pszName, piNameHash );
	if ( iIndex == -1 ) {
		
		DEBUG_CELL cell;
		cell.iPage = 0;
		cell.iRowCount = 1;
		strcpy_s( cell.szName, NAME_SIZE, pszName );
		std::hash<std::string> hash;
		cell.iNameHash = hash(pszName);
		if (piNameHash)
			*piNameHash = cell.iNameHash;
		m_list.push_back( cell );
		iIndex = m_list.size() - 1 ;
	}

	return iIndex;
}

void  CDebugOutput::Set(int iPage, const char* pszName, const char* pszValue, int* piOptContext)
{
	int iIndex = GetCell( iPage, pszName );
	if (iIndex != -1) {
		m_list[iIndex].iPage = iPage;
		sprintf_s(m_list[iIndex].szValue[0], "%-10s", pszValue);
	}
}

void CDebugOutput::Set(int iPage, const char* pszName, int iValue, int* piOptContext)
{
	int iIndex = GetCell( iPage, pszName );
	if (iIndex != -1) {
		m_list[iIndex].iPage = iPage;
		sprintf_s(m_list[iIndex].szValue[0], "% 10d", iValue);
	}
}

void CDebugOutput::Set(int iPage, const char* pszName, float fValue, int iPrec, int* piOptContext)
{
	int iIndex = GetCell( iPage, pszName, piOptContext);
	if ( iIndex != -1 ) {
		m_list[iIndex].iPage = iPage;
		sprintf_s( m_list[iIndex].szValue[0], VALUE_SIZE, "% 10.*f", iPrec, fValue );
	}
}

void CDebugOutput::Set(int iPage, const char* pszName, const vector2& v, int iPrec, int* piOptContext)
{
	int iIndex = GetCell( iPage, pszName );
	if (iIndex != -1) {
		m_list[iIndex].iPage = iPage;
		sprintf_s(m_list[iIndex].szValue[0], VALUE_SIZE, "% 10.*f  % 10.*f", iPrec, v.x, iPrec, v.y);
	}
}

void CDebugOutput::Set(int iPage, const char* pszName, const vector3& v, int iPrec, int* piOptContext)
{
	int iIndex = GetCell( iPage, pszName );
	if (iIndex != -1){
		m_list[iIndex].iPage = iPage;
		sprintf_s(m_list[iIndex].szValue[0], VALUE_SIZE, "% 10.*f  % 10.*f  % 10.*f", iPrec, v.x, iPrec, v.y, iPrec, v.z);
	}
}

void CDebugOutput::Set(int iPage, const char* pszName, const vector4& v, int iPrec, int* piOptContext)
{
	int iIndex = GetCell( iPage, pszName );
	if (iIndex != -1){
		m_list[iIndex].iPage = iPage;
		sprintf_s(m_list[iIndex].szValue[0], VALUE_SIZE, "% 10.*f  % 10.*f  % 10.*f  % 10.*f", iPrec, v.x, iPrec, v.y, iPrec, v.z, iPrec, v.w);
	}
}

void CDebugOutput::Set(int iPage, const char* pszName, const matrix& m, int iPrec, int* piOptContext)
{
	int iIndex = GetCell( iPage, pszName );
	if ( iIndex != -1 ) {
		m_list[iIndex].iPage = iPage;
		char szFormat[] = "% 10.*f  % 10.*f  % 10.*f  % 10.*f";
		sprintf_s( m_list[iIndex].szValue[0], VALUE_SIZE, szFormat, iPrec, m._11, iPrec, m._12, iPrec, m._13, iPrec, m._14 );
		sprintf_s( m_list[iIndex].szValue[1], VALUE_SIZE, szFormat, iPrec, m._21, iPrec, m._22, iPrec, m._23, iPrec, m._24 );
		sprintf_s( m_list[iIndex].szValue[2], VALUE_SIZE, szFormat, iPrec, m._31, iPrec, m._32, iPrec, m._33, iPrec, m._34 );
		sprintf_s( m_list[iIndex].szValue[3], VALUE_SIZE, szFormat, iPrec, m._41, iPrec, m._42, iPrec, m._43, iPrec, m._44 );
		m_list[iIndex].iRowCount = 4;
	}
}

void CDebugOutput::Update( const CRect& rc, int iPage, bool& bVisible, ImVec2 vSizeOnFirstUse, ImGuiWindowFlags flags, float fBgAlpha )
{
	if (!bVisible)
		return;

	//ImGuiWindowFlags flags = 0;
	//flags |= ImGuiWindowFlags_NoTitleBar;
	//flags |= ImGuiWindowFlags_NoMove;
	//flags |= ImGuiWindowFlags_NoInputs;
	//flags |= ImGuiWindowFlags_NoResize;
	//flags |= ImGuiWindowFlags_NoCollapse;

	char szPageCaption[20];
	sprintf_s(szPageCaption, "Page %d", iPage);

	if (ImGui::Begin(szPageCaption, &bVisible, ImVec2(0,0), fBgAlpha, flags)) {

		for (dword i = 0; i < m_list.size(); i++) {

			if (m_list[i].iPage == iPage) {

				ImGui::Columns(2, "table1", false);

				for (int iRow = 0; iRow < m_list[i].iRowCount; iRow++) {
					ImGui::Text(m_list[i].szName);
					ImGui::NextColumn();
					ImGui::Text(m_list[i].szValue[iRow]);
					ImGui::NextColumn();
				}
			}
		}
		ImGui::End();
	}
}

void CDebugOutput::Render(LPD3DXFONT pd3dxFont, CRect& rc, int iPage)
{
	for (dword i = 0; i < m_list.size(); i++) {

		if (m_list[i].iPage == iPage) {

			for (int iRow = 0; iRow < m_list[i].iRowCount; iRow++) {

				char szText[1000];
				sprintf_s(szText, MAX_PATH, "%-15s%s", (iRow == 0) ? m_list[i].szName : "", m_list[i].szValue[iRow]);
				pd3dxFont->DrawTextA(NULL, szText, -1, &rc, DT_LEFT | DT_NOCLIP /*| DT_EXPANDTABS*/, 0xffff0000);
				rc.OffsetRect(0, 20);
			}
		}
	}
}