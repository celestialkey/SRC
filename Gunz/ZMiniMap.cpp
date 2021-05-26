#include "stdafx.h"
#include "ZApplication.h"
#include "ZMiniMap.h"
#include "RealSpace2.h"
#include "MZFileSystem.h"
#include "ZCharacterManager.h"
#include "ZMap.h"
#include "ZGameClient.h"

#define ZMINIMAPTOK_PICTURE		"PICTURE"
#define ZMINIMAPTOK_LEFTTOP		"LEFTTOP"
#define ZMINIMAPTOK_RIGHTBOTTOM	"RIGHTBOTTOM"
#define ZMINIMAPTOK_CAMERAHEIGHT "CAMERAHEIGHT"

#define ZMINIMAP_DEFAULT_CAMERA_HEIGHT_MIN	2000
#define ZMINIMAP_DEFAULT_CAMERA_HEIGHT_MAX	10000

ZMiniMap::ZMiniMap()
{
	m_pBaseTexture = NULL;
	m_pTeamColorTexture = NULL;
	m_pPlayerTexture = NULL;

	m_fCameraHeightMin = ZMINIMAP_DEFAULT_CAMERA_HEIGHT_MIN;
	m_fCameraHeightMax = ZMINIMAP_DEFAULT_CAMERA_HEIGHT_MAX;
}

ZMiniMap::~ZMiniMap()
{
	Destroy();
}

void ZMiniMap::Destroy()
{
	if(m_pBaseTexture)
	{
		RDestroyBaseTexture(m_pBaseTexture);
		m_pBaseTexture = NULL;
	}
	if(m_pTeamColorTexture)
	{
		RDestroyBaseTexture(m_pTeamColorTexture);
		m_pTeamColorTexture = NULL;
	}
	if(m_pPlayerTexture)
	{
		RDestroyBaseTexture(m_pPlayerTexture);
		m_pPlayerTexture = NULL;
	}
}

bool ZMiniMap::Create(const char *szName)
{

	char szXMLName[256];
	char szMapPath[64];
	ZGetCurrMapPath(szMapPath);
	sprintf_safe(szXMLName, "%s%s/%s.minimap.xml", szMapPath, szName, szName);

	MXmlDocument aXml;
	if (!aXml.LoadFromFile(szXMLName, ZApplication::GetFileSystem()))
	{
		return false;
	}

	int iCount, i;
	MXmlElement		aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTextureName[256];

	char szTagName[256],szContents[256];
	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if(stricmp(szTagName,ZMINIMAPTOK_PICTURE)==0) {
			aChild.GetContents(szContents);

			char szMapPath[64];
			ZGetCurrMapPath(szMapPath);
			sprintf(szTextureName, "%s%s/%s", szMapPath, szName,szContents);

		}else
		if(stricmp(szTagName,ZMINIMAPTOK_LEFTTOP)==0) {
			aChild.GetChildContents(&m_LeftTop.x, "X");
			aChild.GetChildContents(&m_LeftTop.y, "Y");
		}else
		if(stricmp(szTagName,ZMINIMAPTOK_RIGHTBOTTOM)==0) {
			aChild.GetChildContents(&m_RightBottom.x, "X");
			aChild.GetChildContents(&m_RightBottom.y, "Y");
		}else
		if(stricmp(szTagName,ZMINIMAPTOK_RIGHTBOTTOM)==0) {
			aChild.GetChildContents(&m_RightBottom.x, "X");
			aChild.GetChildContents(&m_RightBottom.y, "Y");
		}else
		if(stricmp(szTagName,ZMINIMAPTOK_CAMERAHEIGHT)==0) {
			aChild.GetChildContents(&m_fCameraHeightMin, "MIN");
			aChild.GetChildContents(&m_fCameraHeightMax, "MAX");
		}
	}

	m_pBaseTexture = RCreateBaseTexture(szTextureName);
	if(!m_pBaseTexture) return false;

	m_pTeamColorTexture = RCreateBaseTexture("Interface/Combat/minimap_teamcolor.png");
	if(!m_pTeamColorTexture) return false;

	m_pPlayerTexture = RCreateBaseTexture("Interface/Combat/minimap_player.png");
	if(!m_pPlayerTexture) return false;

	return true;
}

void ZMiniMap::OnDraw(MDrawContext* pDC)
{
	if(!m_pBaseTexture) return ;

	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE);

	struct MINIMAPCUSTOMVERTEX{
		FLOAT	x, y, z ;
		FLOAT	tu, tv;
	};

#define D3DFVF_MINIMAPCUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)


	MINIMAPCUSTOMVERTEX Sprite[4] = 
	{
		{ m_LeftTop.x		,m_LeftTop.y	,0,	0,0 },
		{ m_RightBottom.x	,m_LeftTop.y	,0,	1,0 },
		{ m_RightBottom.x	,m_RightBottom.y,0,	1,1 },
		{ m_LeftTop.x		,m_RightBottom.y,0,	0,1 },
	};

	RSetWBuffer(false);

	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);
	
	pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	pd3dDevice->SetFVF(D3DFVF_MINIMAPCUSTOMVERTEX);
	pd3dDevice->SetTexture( 0, m_pBaseTexture->GetTexture() );

	// view , projection 은 이미 설정되어있다는 가정하에,

	rmatrix World;
	D3DXMatrixIdentity(&World);
	pd3dDevice->SetTransform(D3DTS_WORLD, &World);

	RUpdateCamera();

	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(MINIMAPCUSTOMVERTEX));

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_TFACTOR );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );

	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP	, D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP , D3DTOP_DISABLE );

	rboundingbox *pbb = &ZGetGame()->GetWorld()->GetBsp()->GetRootNode()->bbTree;

	MINIMAPCUSTOMVERTEX playerVer[4] = 
	{
		{ -100.f, -200.f ,0,	1,0 },
		{ 100.f	, -200.f ,0,	0,0 },
		{ 100.f	, 200.f	 ,0,	0,1 },
		{ -100.f, 200.f  ,0,	1,1 },
	};

	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		if(pCharacter->IsDie()) continue;

		DWORD color = 0xfffff696;
		if(ZGetGame()->GetMatch()->IsTeamPlay())
			if(pCharacter->GetTeamID()==MMT_RED)
				color = 0xffff8080;
			else color = 0xff8080ff;

		rmatrix World;
		D3DXMatrixIdentity(&World);

		rvector pos = pCharacter->GetPosition();
		pos.z = (pos.z - pbb->minz ) / (pbb->maxz - pbb->minz);
		pos.z = 400.f * min(1.f,max(0.f,pos.z));

		rvector dir = pCharacter->GetDirection();
		dir.z = 0;
		Normalize(dir);

		rvector right;
		rvector up = rvector(0,0,1);

		CrossProduct(&right,up,dir);
		Normalize(right);

		CrossProduct(&up,dir,right);

		World._11 = right.x;	World._12 = dir.x;	World._13 = up.x;
		World._21 = right.y;	World._22 = dir.y;	World._23 = up.y;
		World._31 = right.z;	World._32 = dir.z;	World._33 = up.z;

		// 그림자
		World._41 = pos.x;		World._42 = pos.y;	World._43 = 0;
		pd3dDevice->SetTransform(D3DTS_WORLD, &World);

		pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0xff000000);
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_MODULATE );
		pd3dDevice->SetTexture( 0, m_pTeamColorTexture->GetTexture() );
		pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, playerVer, sizeof(MINIMAPCUSTOMVERTEX));


		World._41 = pos.x;		World._42 = pos.y;	World._43 = pos.z;
		pd3dDevice->SetTransform(D3DTS_WORLD, &World);

		pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, color);
		pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, playerVer, sizeof(MINIMAPCUSTOMVERTEX));

		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_SELECTARG1 );
		pd3dDevice->SetTexture( 0, m_pPlayerTexture->GetTexture() );
		pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, playerVer, sizeof(MINIMAPCUSTOMVERTEX));


	}

	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;

		if (!pCharacter->IsVisible()) continue;
		if (pCharacter->IsDie()) continue;
		
		rvector pos, screen_pos;
		pos = pCharacter->GetPosition();

		rvector dir = pCharacter->GetDirection();
		dir.z = 0;
		Normalize(dir);

		pos += -150.f*dir;
		pos.z = 0;
		screen_pos = RGetTransformCoord(pos);

		MCOLOR _color = MCOLOR(0xfffff696);

		MFont *pFont=NULL;

		if(pCharacter->IsAdminName()) 
	{
		pFont = MFontManager::Get("FONTa12_O1Org");
		int nCount = ( timeGetTime() / 800)% 10; // To change follow this:  999 = time  / 10 = how many colors
		int i = 0;
		for (i; i < nCount;  i++);

		if(i == 0)                             		// To change the colors just change the 6 numbers after 0xff to your colors (hex)
		pDC->SetColor(0xffff0000);			// Red
		if(i == 1)
		pDC->SetColor(0xff00ffff);			// Cyan
		if(i == 2)
		pDC->SetColor(0xffe0a03c);			// Orange
		if(i == 3)
		pDC->SetColor(0xffffff00);			// Yellow
		if(i == 4)
		pDC->SetColor(0xff00ff00);			// Green
		if(i == 5)
		pDC->SetColor(0xff0078ff);			// Dark Blue
		if(i == 6)
		pDC->SetColor(0xffb267ec);			// Purple
		if(i == 7)
		pDC->SetColor(0xffd5ff00);			// Lime
		if(i == 8)
		pDC->SetColor(0xff68C8FF);			// Light Blue
		if(i == 9)
		pDC->SetColor(0xffff00ff);			// Pink
	}
		else if(pCharacter->IsRootName()) 
	{
		pFont = MFontManager::Get("FONTa12_O1Org");
		int nCount = ( timeGetTime() / 800)% 10; // To change follow this:  999 = time  / 10 = how many colors
		int i = 0;
		for (i; i < nCount;  i++);

		if(i == 0)                             		// To change the colors just change the 6 numbers after 0xff to your colors (hex)
		pDC->SetColor(0xffff0000);			// Red
		if(i == 1)
		pDC->SetColor(0xff00ffff);			// Cyan
		if(i == 2)
		pDC->SetColor(0xffe0a03c);			// Orange
		if(i == 3)
		pDC->SetColor(0xffffff00);			// Yellow
		if(i == 4)
		pDC->SetColor(0xff00ff00);			// Green
		if(i == 5)
		pDC->SetColor(0xff0078ff);			// Dark Blue
		if(i == 6)
		pDC->SetColor(0xffb267ec);			// Purple
		if(i == 7)
		pDC->SetColor(0xffd5ff00);			// Lime
		if(i == 8)
		pDC->SetColor(0xff68C8FF);			// Light Blue
		if(i == 9)
		pDC->SetColor(0xffff00ff);			// Pink
	}
		else if(pCharacter->IsOpName()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_OP_NAME));
				}
				else if(pCharacter->IsDeveloperName()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_DEVELOPER_NAME));
				}
				else if(pCharacter->IsEventMasterName()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_EVENTMASTER_NAME));
				}
				else if(pCharacter->IsMuteName()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_MUTE_NAME));
				}
				else if(pCharacter->IsVipName()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP_NAME));
				}
				else if(pCharacter->IsVIP1Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP1_NAME));
				}
				else if(pCharacter->IsVIP2Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP2_NAME));
				}
				else if(pCharacter->IsVIP3Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP3_NAME));
				}
				else if(pCharacter->IsVIP4Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP4_NAME));
				}
				else if(pCharacter->IsVIP5Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP5_NAME));
				}
				else if(pCharacter->IsVIP6Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP6_NAME));
				}
				else if(pCharacter->IsVIP7Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP7_NAME));
				}
				else if(pCharacter->IsVIP8Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP8_NAME));
				}
				else if(pCharacter->IsVIP9Name()) {
					pFont = MFontManager::Get("FONTa12_O1Org");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP9_NAME));
				}	
				else if(pCharacter->IsVIP10Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP10_NAME));
				}
				else if(pCharacter->IsVIP11Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP11_NAME));
				}
				else if(pCharacter->IsVIP12Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP12_NAME));
				}
				else if(pCharacter->IsVIP13Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP13_NAME));
				}
				else if(pCharacter->IsVIP14Name()) {
					pFont = MFontManager::Get("FONTa12_O1Red");
					pDC->SetColor(MCOLOR(ZCOLOR_VIP14_NAME));
				}
				else if(pCharacter->IsVIP15Name()) 
	{
		pFont = MFontManager::Get("FONTa12_O1Red");
		int i = 0;
		int nCount = ( timeGetTime() / 500)% 14; // To change follow this:  999 = time  / 10 = how many colors
		for ( i; i < nCount;  i++);

		if(i == 0)                             		// To change the colors just change the 6 numbers after 0xff to your colors (hex)
		pDC->SetColor(0xFFFF0080);			// Orange
		if(i == 1)
		pDC->SetColor(0xFF0080FF);			// Purple
		if(i == 2)
		pDC->SetColor(0xFFFF8000);			// Red	
		if(i == 3)
		pDC->SetColor(0xFFFF00FF);			// Yellow
		if(i == 4)
		pDC->SetColor(0xFF12E29D);			// Pink			
		if(i == 5)
		pDC->SetColor(0xFF48d1cc);			// Dark Blue
		if(i == 6)
		pDC->SetColor(0xFFFFFF00);			// Cyan			
		if(i == 7)
		pDC->SetColor(0xFF0000FF);			// Lime
		if(i == 8)
		pDC->SetColor(0xFF86CB2C);			// Light Blue
		if(i == 9)
		pDC->SetColor(0xFF8000FF);			// Green
		if(i == 10)
		pDC->SetColor(0xFF2EC56E);			// Pink
		if(i == 11)
		pDC->SetColor(0xFF5C1E08);			// Pink
		if(i == 12)
		pDC->SetColor(0xFFFFFFFF);			// Pink
		if(i == 13)
		pDC->SetColor(0xFF000000);			// Pink
	}
		else {
			pFont = MFontManager::Get("FONTa12_O1Blr");
			if(ZGetGame()->GetMatch()->IsTeamPlay())
				if(pCharacter->GetTeamID()==MMT_RED)
					pFont = MFontManager::Get("FONTa12_O1Red");
			pDC->SetColor(_color);
		}

//		pFont = MFontManager::Get("FONTa10b");
		pDC->SetBitmap(NULL);

		/////// Outline Font //////////
//				MFont *pFont=MFontManager::Get("FONTa12_O1Blr");
		pDC->SetFont(pFont);
		///////////////////////////////

		int x = screen_pos.x - pDC->GetFont()->GetWidth(pCharacter->GetUserName()) / 2;
		int y = screen_pos.y;// + MGetWorkspaceHeight()*0.05;

		/*
		if(ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter() == pCharacter) {

			int nWidth = pDC->GetFont()->GetWidth(pCName);
			pDC->SetColor(MCOLOR(0xffffffff));

#define MARGIN	5
			pDC->FillRectangle(x-MARGIN,y-MARGIN,nWidth+MARGIN*2,pDC->GetFont()->GetHeight()+MARGIN*2);
		}
		*/

		pDC->Text(x, y, pCharacter->GetUserName());
	}
}