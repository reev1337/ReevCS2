#include "legitbot.h"

// used: movement callback
#include "legitbot/aim.h"

void F::LEGIT::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	AMIM::OnMove(pCmd, pBaseCmd, pLocalController, pLocalPawn);
}
