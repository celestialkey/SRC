#include "stdafx.h"
#include "MBMatchServer.h"
#include "MBMatchNHNAuth.h"
#include "MBMatchAsyncDBJob_NHNLogin.h"
#include "MBMatchAsyncDBJob_GameOnLogin.h"
#include "MBMatchAsyncDBJob_NetmarbleLogin.h"
#include "MMatchConfig.h"
#include "MMatchLocale.h"
#include "MMatchAuth.h"
#include "MBMatchAuth.h"
#include "MMatchStatus.h"
#include "MMatchGlobal.h"
#include "MBMatchGameOnAuth.h"


void MBMatchServer::OnRequestLoginNetmarble(const MUID& CommUID, const char* szAuthCookie, const char* szDataCookie, const char* szCPCookie, const char* szSpareData, int nCmdVersion, unsigned long nChecksumPack)
{
	return;
	
}


void MBMatchServer::OnRequestLoginNHNUSA( const MUID& CommUID, const char* pszUserID, const char* pszAuthStr, const int nCommandVersion, const int nCheckSumPack, char* szEncryptMd5Value )
{
	return;
}

void MBMatchServer::OnRequestLoginGameOn( const MUID& CommUID, const char* szString, const char* szStatIndex, int nCommandVersion, int nCheckSumPack, char* szEncryptMd5Value )
{
	return;
}


void MBMatchServer::OnRequestAccountCharList(const MUID& uidPlayer, unsigned char* pbyGuidAckMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
	{
		DWORD dwRet = HShield_AnalyzeGuidAckMsg(pbyGuidAckMsg, pObj->GetHShieldInfo()->m_pbyGuidReqInfo, 
			&pObj->GetHShieldInfo()->m_pCRCInfo);

		if(dwRet!= ERROR_SUCCESS)
		{
			pObj->GetHShieldInfo()->m_bGuidAckPass = false;
			LOG(LOG_FILE, "@AnalyzeGuidAckMsg - Find Hacker(%s) : (Error Code = %x)", 
				pObj->GetAccountName(), dwRet);

			MCommand* pNewCmd = CreateCommand(MC_MATCH_FIND_HACKING, MUID(0,0));
			RouteToListener(pObj, pNewCmd);

#ifndef _DEBUG
			// �������� �����̹Ƿ� ĳ���� �����Ҷ� ������ ���´�.
			pObj->SetHacker(true);
#endif
		}
		else
		{
			pObj->GetHShieldInfo()->m_bGuidAckPass = true;
			if(pObj->GetHShieldInfo()->m_pCRCInfo == NULL)
				mlog("%s's HShield_AnalyzeGuidAckMsg Success. but pCrcInfo is NULL...\n", pObj->GetAccountName());
		}
//		SendHShieldReqMsg();
	}
#endif

	const MMatchHackingType MHackingType = pObj->GetAccountInfo()->m_HackingType;
	
	if( MGetServerConfig()->IsBlockHacking() && (MMHT_NO != MHackingType) && !IsAdminGrade(pObj) )
	{
		// ���⼭ ���� �ð� ���Ḧ �˻��� ��� �Ѵ�.
		if( !IsExpiredBlockEndTime(pObj->GetAccountInfo()->m_EndBlockDate) )
		{
			DWORD dwMsgID = 0;

			if( MMHT_XTRAP_HACKER == MHackingType)				 dwMsgID = MERR_BLOCK_HACKER;
			else if(MMHT_HSHIELD_HACKER == MHackingType)		 dwMsgID = MERR_BLOCK_HACKER;
			else if(MMHT_BADUSER == MHackingType)				 dwMsgID = MERR_BLOCK_BADUSER;			
			else if(MMHT_COMMAND_FLOODING == MHackingType)		 dwMsgID = MERR_FIND_FLOODING;
			else if(MMHT_GAMEGUARD_HACKER == MHackingType) {}
			else
				dwMsgID = MERR_FIND_HACKER;

			pObj->GetDisconnStatusInfo().SetMsgID( dwMsgID );
			pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
			return;
		}
		else if( MMHT_SLEEP_ACCOUNT == MHackingType ) 
		{
			pObj->GetDisconnStatusInfo().SetMsgID( MERR_BLOCK_SLEEP_ACCOUNT );
			pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
			return;
		}
		else
		{
			// �Ⱓ�� ����Ǿ����� DB�� ���������� ������Ʈ ���ش�.
			pObj->GetAccountInfo()->m_HackingType = MMHT_NO;

			MAsyncDBJob_ResetAccountHackingBlock* pResetBlockJob = new MAsyncDBJob_ResetAccountHackingBlock(uidPlayer);
			pResetBlockJob->Input( pObj->GetAccountInfo()->m_nAID, MMHT_NO );

			// PostAsyncJob( pResetBlockJob );
			pObj->m_DBJobQ.DBJobQ.push_back( pResetBlockJob );
		}
	}

	if( pObj->GetAccountPenaltyInfo()->IsBlock(MPC_CONNECT_BLOCK) ) {
		pObj->GetDisconnStatusInfo().SetMsgID( MERR_BLOCK_BADUSER );
		pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
		return;
	}

	// Async DB //////////////////////////////
	pObj->UpdateTickLastPacketRecved();

	// ����Ʈ ��尡 �ƴϸ� ������Ʈ�� �����Ͱ� ����. -- by SungE. 2006-11-07
	if( MSM_TEST == MGetServerConfig()->GetServerMode() )
	{
		if( 0 != pObj->GetCharInfo() )
		{
			MAsyncDBJob_UpdateQuestItemInfo* pQItemUpdateJob = new MAsyncDBJob_UpdateQuestItemInfo(pObj->GetUID());
			if( 0 == pQItemUpdateJob )
				return;

			if( !pQItemUpdateJob->Input(pObj->GetCharInfo()->m_nCID, 
				pObj->GetCharInfo()->m_QuestItemList, 
				pObj->GetCharInfo()->m_QMonsterBible) )
			{
				mlog( "MMatchServer::OnAsyncGetAccountCharList - ��ü ���� ����.\n" );
				delete pQItemUpdateJob;
				return;
			}

			// ���⼭ ����Ʈ ������Ʈ ������ �Ǹ� CharFinalize���� �ߺ� ������Ʈ�� �ɼ� �ֱ⿡,
			//  CharFinalize���� �Ʒ� �÷��װ� true�� ������Ʈ�� �����ϰ� �س�����.
			pObj->GetCharInfo()->m_QuestItemList.SetDBAccess( false );

			// PostAsyncJob( pQItemUpdateJob );
			pObj->m_DBJobQ.DBJobQ.push_back( pQItemUpdateJob );
		}
	}

	MAsyncDBJob_GetAccountCharList* pJob=new MAsyncDBJob_GetAccountCharList(uidPlayer,pObj->GetAccountInfo()->m_nAID);
	// PostAsyncJob(pJob);
	pObj->m_DBJobQ.DBJobQ.push_back( pJob );
}