#pragma once

#include <tlCBaseGuiWnd.h>

class C3DViewContext;

class CSettingsGui : public CBaseGuiWnd
{
public:
	CSettingsGui();
	~CSettingsGui();
	bool Initialize();
	void CleanUp();
	void Update(C3DViewContext* pView);
	void OnShow(bool bShow);
	void ResetToDefaults();
	float GetAlpha() { return m_fAlpha; }
	bool GetShowFpsFlag() { return m_bShowFps; }

private:
	float	m_fAlpha;
	bool	m_bShowFps;
	bool	m_bFirstTime;
};

