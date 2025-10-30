#include "aim.h"
#include <Windows.h>

// used: sdk entity
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/iengineclient.h"
// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"
#include "../../core/sdk.h"
// used: activation button
#include "../../utilities/inputsystem.h"

// used: cheat variables
#include "../../core/variables.h"

#include "../../sdk/interfaces/cgametracemanager.h"

void F::LEGIT::AMIM::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	// Check if the legitbot is enabled
	if (!C_GET(bool, Vars.bLegitbot))
		return;

	if (!pLocalController->IsPawnAlive())
		return;

	AimAssist(pBaseCmd, pLocalPawn, pLocalController);
	Silent(pCmd, pBaseCmd, pLocalController, pLocalPawn);
}

QAngle_t GetRecoil(CBaseUserCmdPB* pCmd, C_CSPlayerPawn* pLocal)
{
	static QAngle_t OldPunch; // Store last tick's AimPunch angles
	if (pLocal->GetShotsFired() >= 1) // Only update AimPunch while shooting
	{
		QAngle_t viewAngles = pCmd->pViewAngles->angValue;
		QAngle_t delta = viewAngles - (viewAngles + (OldPunch - (pLocal->GetAimPuchAngle() * 2.f))); // Calculate current AimPunch angles delta

		return pLocal->GetAimPuchAngle() * 2.0f; // Return corrected AimPunch delta
	}
	else
	{
		return QAngle_t{ 0, 0, 0 }; // Return zero if not shooting
	}
}

QAngle_t GetAngularDifference(CBaseUserCmdPB* pCmd, const Vector_t& vecTarget, C_CSPlayerPawn* pLocal)
{
	Vector_t vecCurrent = pLocal->GetEyePosition();
	QAngle_t vNewAngle = (vecTarget - vecCurrent).ToAngles();
	vNewAngle.Normalize();

	QAngle_t vCurAngle = pCmd->pViewAngles->angValue;
	vNewAngle -= vCurAngle;

	return vNewAngle;
}

float GetAngularDistance(CBaseUserCmdPB* pCmd, const Vector_t& vecTarget, C_CSPlayerPawn* pLocal)
{
	return GetAngularDifference(pCmd, vecTarget, pLocal).Length2D();
}

void MouseClick()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &input, sizeof(INPUT));
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(INPUT));
}

void MouseClick2()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &input, sizeof(INPUT));
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(INPUT));
}

void F::LEGIT::AMIM::AimAssist(CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn, CCSPlayerController* pLocalController)
{
	if (!IPT::IsKeyDown(C_GET(unsigned int, Vars.nLegitbotActivationKey)) && !C_GET(bool, Vars.bLegitbotAlwaysOn))
		return;
	float flDistance = INFINITY;
	CCSPlayerController* pTarget = nullptr;
	Vector_t vecBestPosition = Vector_t();

	const int iHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();

	for (int nIndex = 1; nIndex <= iHighestIndex; nIndex++)
	{
		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(nIndex);
		if (pEntity == nullptr)
			continue;

		SchemaClassInfoData_t* pClassInfo = nullptr;
		pEntity->GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			continue;

		const FNV1A_t uHashedName = FNV1A::Hash(pClassInfo->szName);

		if (uHashedName != FNV1A::HashConst("CCSPlayerController"))
			continue;

		CCSPlayerController* pPlayer = reinterpret_cast<CCSPlayerController*>(pEntity);
		if (pPlayer == nullptr)
			continue;

		if (pPlayer == pLocalController)
			continue;

		C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
		if (pPawn == nullptr)
			continue;

		if (!pPlayer->IsPawnAlive())
			continue;

		if (!pLocalPawn->IsOtherEnemy(pPawn))
			continue;

		// Check if they're dormant
		CGameSceneNode* pCGameSceneNode = pPawn->GetGameSceneNode();
		if (pCGameSceneNode == nullptr || pCGameSceneNode->IsDormant())
			continue;

		// Get the skeleton
		CSkeletonInstance* pSkeleton = pCGameSceneNode->GetSkeletonInstance();
		if (pSkeleton == nullptr)
			continue;

		// Get the bones
		Matrix2x4_t* pBoneCache = pSkeleton->pBoneCache;
		if (pBoneCache == nullptr)
			continue;

		// const int iBone = C_GET(int, Vars.iTargetBone);

		const int iBone = 6; // Target the head bone

		// Get the bone's position
		Vector_t vecPos = pBoneCache->GetOrigin(iBone);

		// Initialize trace, construct filter, and initialize ray
		GameTrace_t trace = GameTrace_t();
		TraceFilter_t filter = TraceFilter_t(0x1C3003, pLocalPawn, nullptr, 4);
		Ray_t ray = Ray_t();

		// Cast a ray from local player eye position to player head bone
		I::GameTraceManager->TraceShape(&ray, pLocalPawn->GetEyePosition(), vecPos, &filter, &trace);

		// Check if the hit entity is the one we wanted to check and if the trace end point is visible
		if (trace.m_pHitEntity != pPawn || !trace.IsVisible())
			continue;

		// Get the distance/weight of the move
		float flCurrentDistance = GetAngularDistance(pUserCmd, vecPos, pLocalPawn);
		if (flCurrentDistance > C_GET(float, Vars.flAimRange))
			continue;
		if (pTarget && flCurrentDistance > flDistance)
			continue;

		// Better move found, override
		pTarget = pPlayer;
		flDistance = flCurrentDistance;
		vecBestPosition = vecPos;
	}

	// Check if a target was found
	if (pTarget == nullptr)
		return;

	// Point at them
	QAngle_t* pViewAngles = &(pUserCmd->pViewAngles->angValue);

	// Find the change in angles
	QAngle_t vNewAngles = GetAngularDifference(pUserCmd, vecBestPosition, pLocalPawn);

	// Get the smoothing
	const float flSmoothing = C_GET(float, Vars.flSmoothing);
	auto aimPunch = GetRecoil(pUserCmd, pLocalPawn); // Get AimPunch angles

	// Apply smoothing and set angles
	pViewAngles->x += (vNewAngles.x - aimPunch.x) / flSmoothing; // Subtract AimPunch angle to counteract recoil
	pViewAngles->y += (vNewAngles.y - aimPunch.y) / flSmoothing;
	pViewAngles->Normalize();

	// Triggerbot logic
	if (C_GET(bool, Vars.trigger))
	{
		// Check if the target is in the crosshair
		if (flDistance <= C_GET(float, Vars.flTriggerbotFov))
		{
			MouseClick();
		}
	}
}

void F::LEGIT::AMIM::Silent(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!IPT::IsKeyDown(C_GET(unsigned int, Vars.nLegitbotActivationKey)) && !C_GET(bool, Vars.Silent))
		return;

	float flDistance = INFINITY;
	CCSPlayerController* pTarget = nullptr;
	Vector_t vecBestPosition;

	const int iHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();

	for (int nIndex = 1; nIndex <= iHighestIndex; ++nIndex)
	{
		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(nIndex);
		if (pEntity == nullptr)
			continue;

		SchemaClassInfoData_t* pClassInfo = nullptr;
		pEntity->GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			continue;

		const FNV1A_t uHashedName = FNV1A::Hash(pClassInfo->szName);
		if (uHashedName != FNV1A::HashConst("CCSPlayerController"))
			continue;

		CCSPlayerController* pPlayer = reinterpret_cast<CCSPlayerController*>(pEntity);
		if (pPlayer == nullptr)
			continue;

		if (pPlayer == pLocalController)
			continue;

		C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
		if (pPawn == nullptr)
			continue;

		if (!pPlayer->IsPawnAlive())
			continue;

		if (!pLocalPawn->IsOtherEnemy(pPawn))
			continue;

		CGameSceneNode* pCGameSceneNode = pPawn->GetGameSceneNode();
		if (pCGameSceneNode == nullptr || pCGameSceneNode->IsDormant())
			continue;

		CSkeletonInstance* pSkeleton = pCGameSceneNode->GetSkeletonInstance();
		if (pSkeleton == nullptr)
			continue;

		Matrix2x4_t* pBoneCache = pSkeleton->pBoneCache;
		if (pBoneCache == nullptr)
			continue;

		const int iBone = 6;
		Vector_t vecPos = pBoneCache->GetOrigin(iBone);
		float flCurrentDistance = GetAngularDistance(pBaseCmd, vecPos, pLocalPawn);
		if (pTarget && flCurrentDistance > flDistance)
			continue;

		pTarget = pPlayer;
		flDistance = flCurrentDistance;
		vecBestPosition = vecPos;
		/*if (C_GET(bool, Vars.trigger))
        {
            if (flDistance <= C_GET(float, Vars.flTriggerbotFov))
            {
                MouseClick2();
            }
        }*/
	}

	if (pTarget == nullptr)
		return;

	QAngle_t vNewAngles = GetAngularDifference(pBaseCmd, vecBestPosition, pLocalPawn);

	for (int i = 0; i < pCmd->csgoUserCmd.inputHistoryField.nCurrentSize; ++i)
	{
		CCSGOInputHistoryEntryPB* pInputEntry = pCmd->GetInputHistoryEntry(i);
		if (!pInputEntry || !pInputEntry->pViewAngles)
			continue;

		pInputEntry->pViewAngles->angValue += vNewAngles;
		pInputEntry->pViewAngles->angValue.Normalize();
		pInputEntry->SetBits(EInputHistoryBits::INPUT_HISTORY_BITS_VIEWANGLES);
	}
}

void F::LEGIT::AMIM::Draw()
{
	if (!C_GET(bool, Vars.AimFov))
		return;

	if (SDK::LocalPawn == 0)
		return;

	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	ImVec2 screenCenter = ImVec2(
	ImGui::GetIO().DisplaySize.x / 2.0f,
	ImGui::GetIO().DisplaySize.y / 2.0f);

	float aimFov = C_GET(float, Vars.flAimRange);

	float maxFov = 15.0f; // Maximum FOV
	float screenHeight = ImGui::GetIO().DisplaySize.y; // Screen height
	float baseScaleFactor = screenHeight / (2.0f * tanf(maxFov * 0.5f * (3.14159265f / 180.0f)));

	float radius = tanf(aimFov * 0.5f * (3.14159265f / 180.0f)) * baseScaleFactor;

	// Reduce radius to avoid being too large
	float scaleAdjustment = 0.35f; // Multiplier to reduce radius
	radius *= scaleAdjustment;

	Color_t color = C_GET(Color_t, Vars.ColorFoV);

	// Convert Color_t to IM_COL32 (ABGR order)
	ImU32 imColor = IM_COL32(
	static_cast<int>(color.r * 255), // R
	static_cast<int>(color.g * 255), // G
	static_cast<int>(color.b * 255), // B
	static_cast<int>(color.a * 255) // A
	);

	// Draw the circle with the current radius
	drawList->AddCircle(screenCenter, radius, imColor, 100); // Use the converted color
}
