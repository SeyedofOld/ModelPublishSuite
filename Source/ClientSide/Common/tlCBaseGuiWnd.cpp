#include "stdafx.h"
#include "tlCBaseGuiWnd.h"


CBaseGuiWnd::CBaseGuiWnd()
{
	m_bVisible = false;

	m_windowRect.x1 = 0;
	m_windowRect.y1 = 0;
	m_windowRect.x2 = 0;
	m_windowRect.y2 = 0;
	m_fWndWidth = 0;
	m_fWndHeight = 0;
}

CBaseGuiWnd::~CBaseGuiWnd()
{
}

void CBaseGuiWnd::SetWndRect(const MYRECT& rc)
{
	m_windowRect = rc;
	m_fWndWidth = rc.x2 - rc.x1;
	m_fWndHeight = rc.y2 - rc.y1;
}

void CBaseGuiWnd::SetWndRect(float x1, float y1, float x2, float y2)
{
	m_windowRect.x1 = x1;
	m_windowRect.y1 = y1;
	m_windowRect.x2 = x2;
	m_windowRect.y2 = y2;
	m_fWndWidth = x2 - x1;
	m_fWndHeight = y2 - y1;
}

bool CBaseGuiWnd::Inside(const tochal::vector2& pt)
{
	if (pt.x < m_windowRect.x1)
		return false;
	if (pt.x > m_windowRect.x2)
		return false;
	if (pt.y < m_windowRect.y1)
		return false;
	if (pt.y > m_windowRect.y2)
		return false;

	return true;
}

void CBaseGuiWnd::Show(bool bShow)
{
	if (bShow != m_bVisible)
		OnShow(bShow);
	m_bVisible = bShow;
}

void CBaseGuiWnd::OnShow(bool bShow)
{

}