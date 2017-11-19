#pragma once

class CBaseGuiWnd
{
public:

	struct MYRECT {
		float x1, y1, x2, y2;
	};
	
	CBaseGuiWnd();
	~CBaseGuiWnd();
	void SetWndRect(const MYRECT& rc);
	void SetWndRect(float x1, float y1, float x2, float y2);
	MYRECT& GetWindowRect() { return m_windowRect; }
	float GetWindowWidth() { return m_fWndWidth; }
	float GetWindowHeight() { return m_fWndHeight; }
	bool Inside(const tochal::vector2& pt);
	void Show(bool bShow);
	bool IsWindowVisible() { return m_bVisible; }
	virtual void OnShow(bool bShow);

	MYRECT	m_windowRect;
	float	m_fWndWidth;
	float	m_fWndHeight;
	bool m_bVisible;
};

