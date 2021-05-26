#pragma once

#include <map>

#include "MTypes.h"
#include "MEvent.h"

class MDrawContext;
class MWidget;
class MFont;
class MBitmap;
class MResourceMap;
class MIDLResource;
class MListener;
class MEvent; 

#define MINT_VERSION	2
#define	MVersion()	MINT_VERSION

typedef bool(MGLOBALEVENTCALLBACK)(MEvent* pEvent);

class Mint{
protected:
	static Mint*			m_pInstance;	///< Instance
	MWidget*				m_pMainFrame;	///< Main Frame Widget
	MDrawContext*			m_pDC;			///< DrawContext
	MGLOBALEVENTCALLBACK*	m_fnGlobalEventCallBack;	///< Global Event Callback function

	char		m_szDragObjectString[256];
	char		m_szDragObjectItemString[256];
	MBitmap*	m_pDragObjectBitmap;
	MPOINT		m_GrabPoint;
	bool		m_bVisibleDragObject;
	MWidget*	m_pDropableObject;
	MWidget*	m_pDragSourceObject;

	// Workspace Size
	int			m_nWorkspaceWidth;
	int			m_nWorkspaceHeight;
	bool		Stretch = true;


	void*	m_pCandidateList;		
	int		m_nCandidateListSize;	
	MPOINT	m_CandidateListPos;	

	bool	m_bEnableIME;

public:
	DWORD	m_nCompositionAttributeSize;		
	BYTE	m_nCompositionAttributes[MIMECOMPOSITIONSTRING_LENGTH];
	int		m_nCompositionCaretPosition;

protected:
	virtual void DrawCandidateList(MDrawContext* pDC, MPOINT& p);

public:
	Mint(void);
	virtual ~Mint(void);

	/// Mint Initialize
	bool Initialize(int nWorkspaceWidth, int nWorkspaceHeight, MDrawContext* pDC, MFont* pDefaultFont);
	/// Mint Finalize
	void Finalize(void);

#ifdef WIN32
	/// Event Processing
	bool ProcessEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
#endif

	/// Mint Run
	virtual void Run(void);
	/// Mint Draw
	virtual void Draw(void);

	/// Event Action Key
//	bool EventActionKey(unsigned long int nKey, bool bPressed);

	/// Update(Redraw)
	virtual void Update(void){}

	/// Get MainFrame Widget
	MWidget* GetMainFrame(void);

	/// Get DrawContext
	MDrawContext* GetDrawContext(void);

	/// Singleton Instance
	static Mint* GetInstance(void);

#ifdef WIN32
	// ������ �ڵ� ����
	void SetHWND(HWND hWnd);
	HWND GetHWND(void);

	HIMC m_hImc;			// IME Handle, IME�� Enable/Disable�ϱ� ���� ������. IME���� �Լ��� ������ ImmGetContext()�� �� ����Ѵ�. ����, Disable�Ǿ� ������, ImmGetContext()�� NULL�� ������ ���̴�.
	//DWORD m_dwIMEConvMode;	// ������ IME ���
#endif

	void EnableIME(bool bEnable);
	bool IsEnableIME(void);

	/// ��Ű �߰�
	int RegisterHotKey(unsigned long int nModifier, unsigned long int nVirtKey);
	/// ��Ű ����
	void UnregisterHotKey(int nID);



	// Drag & Drop
	MWidget* SetDragObject(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
	MWidget* GetDragObject(void);

	virtual MWidget* NewWidget(const char* szClass, const char* szName, MWidget* pParent, MListener* pListener);

	MWidget* FindWidgetDropAble(MPOINT& p);
	MWidget* FindWidget(MPOINT& p);
	MWidget* FindWidget(int x, int y);

	int GetWorkspaceWidth();
	int GetWorkspaceHeight();
	void SetWorkspaceSize(int w, int h);

	auto GetStretch() const { return Stretch; }
	void SetStretch(bool b) { Stretch = b; }


	/// �ý��ۿ� �´� Bitmap Open Function�� �����ؾ� �Ѵ�.
	virtual MBitmap* OpenBitmap(const char* szName) = 0;
	/// �ý��ۿ� �´� Font Open Function�� �����ؾ� �Ѵ�.
	virtual MFont* OpenFont(const char* szName, int nHeight) = 0;

	void SetGlobalEvent(MGLOBALEVENTCALLBACK pGlobalEventCallback);

	// IME ���� �Լ���
	/// ���� �� ���� �⺻ ��Ʈ ���
	const char* GetDefaultFontName(void) const;

	/// �� ��� ID ���
	/// LANG_KOREAN, LANG_JAPANESE, LANG_CHINESE, etc... in "winnt.h"
	int GetPrimaryLanguageIdentifier(void) const;

	/// �� ��� ID ���
	/// �߱��� ��ü, ��ü�� ��� ���ؼ��� �� ������ �ʿ�
	/// SUBLANG_KOREAN, SUBLANG_CHINESE_TRADITIONAL, SUBLANG_CHINESE_SIMPLIFIED, ... in "winnt.h"
	int GetSubLanguageIdentifier(void) const;

	/// �ε�������(En, ��, ��, �� ��) ���
	const char* GetLanguageIndicatorString(void) const;

	/// Native ���� �����Ǿ� �ִ°�?
	bool IsNativeIME(void) const;

	/// WM_IME_NOTIFY�� IMN_OPENCANDIDATE ����, Candidate List �˾��� �� ȣ��
	void OpenCandidateList(void);
	/// WM_IME_NOTIFY�� IMN_CLOSECANDIDATE ����, Candidate List �˾��� ���� �� ȣ��
	void CloseCandidateList(void);

	// Candidate List ���� �Լ���
	/// �ε��������� Candidate ���ڿ� ���
	const char* GetCandidate(int nIndex) const;
	/// Candidate ���ڿ� �� ���� ���
	int GetCandidateCount(void) const;
	/// Candidate ���ڿ��� �� ���� ���õ� ���ڿ� �ε��� ���
	int GetCandidateSelection(void) const;
	/// ���� �������� ������ Candidate ���ڿ��� ù��° �ε��� ���
	int GetCandidatePageStart(void) const;
	/// �� �������� ������ Candidate ���ڿ��� ���� ���
	int GetCandidatePageSize(void) const;

	/// Candidate List�� ������ ��ġ �����ϱ�
	/// p�� ���յǴ� ������ ���� ��ġ
	/// �Ʒ��� ������ �����ϸ� ���� �������� �ϹǷ� ������ ���̸� �˾ƾ� �Ѵ�.
	void SetCandidateListPosition(MPOINT& p, int nWidgetHeight);
	/// Candidate List �ʺ�
	int GetCandidateListWidth(void);
	/// Candidate List ����
	int GetCandidateListHeight(void);

	/// Composition �Ӽ� ���� ���
	DWORD GetCompositionAttributeSize(void) const { return m_nCompositionAttributeSize; }
	/// Composition �Ӽ� ���
	const BYTE* GetCompositionAttributes(void) const { return m_nCompositionAttributes; }
	//const DWORD* GetCompositionClauses(void) const { return m_dwCompositionClauses; }

	/// �������� �Ӽ� �׸���
	/// p�� szComposition[i]�� ��ġ
	int DrawCompositionAttribute(MDrawContext* pDC, MPOINT& p, const char* szComposition, int i);
	/// p�� Composition�� �����ϴ� ��ġ
	void DrawCompositionAttributes(MDrawContext* pDC, MPOINT& p, const char* szComposition);
	/// ���ʿ� �ε������� �׸���
	void DrawIndicator(MDrawContext* pDC, MRECT& r);
};

inline int MGetWorkspaceWidth() 
{
	return Mint::GetInstance()->GetWorkspaceWidth();
}
inline int MGetWorkspaceHeight()
{
	return Mint::GetInstance()->GetWorkspaceHeight();
}


inline int MGetCorrectedWorkspaceWidth()
{
	if (Mint::GetInstance()->GetStretch())
		return MGetWorkspaceWidth();

	auto Aspect = static_cast<float>(MGetWorkspaceWidth()) / MGetWorkspaceHeight();
	return static_cast<int>(MGetWorkspaceWidth() / Aspect * (4.f / 3.f));
}


#define CONVERT800(x)	(int((x)/800.f * MGetWorkspaceWidth()))
#define CONVERT600(y)	(int((y)/600.f * MGetWorkspaceHeight()))

// Sample
void MCreateSample(void);
void MDestroySample(void);



#define MMODIFIER_ALT	1
#define MMODIFIER_CTRL	2
#define MMODIFIER_SHIFT	4

#define MIsActionKeyPressed(_ActionID)	(Mint::GetInstance()->IsActionKeyPressed(_ActionID))

#define IsHangul(x) ((unsigned char)(x)>127)
