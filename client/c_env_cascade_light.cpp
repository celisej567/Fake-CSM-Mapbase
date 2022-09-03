//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "shareddefs.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "texture_group_names.h"
#include "tier0/icommandline.h"
#include "vguicenterprint.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVarRef mat_slopescaledepthbias_shadowmap("mat_slopescaledepthbias_shadowmap");
static ConVarRef mat_depthbias_shadowmap("mat_depthbias_shadowmap");
static ConVar scissor("r_flashlightscissor", "0");

#ifdef MAPBASE
static ConVar csm_ortho("csm_ortho","0", 0, "Turn light into ortho. Im lazy right now to make this works fine");
ConVar csm_ortho_nearz("csm_ortho_nearz", "512");
ConVar csm_ortho_left("csm_ortho_left", "-1000");
ConVar csm_ortho_top("csm_ortho_top", "-1000");
ConVar csm_ortho_bottom("csm_ortho_bottom", "1000");
ConVar csm_ortho_right("csm_ortho_right", "1000");
//ConVar csm_test_color_interpolation("csm_test_color_interpolation","0"); //я не помню что она делает, но она определённо этого не делает
#endif

//-----------------------------------------------------------------------------
// Purpose: main point for change angle of the light
//-----------------------------------------------------------------------------


class C_LightOrigin : public C_BaseEntity
{
	DECLARE_CLASS(C_LightOrigin, C_BaseEntity);
public:
	DECLARE_CLIENTCLASS();

	void Update();

	virtual void Simulate();

private:
	Vector LightEnvVector;
	QAngle LightEnvAngle;
};

IMPLEMENT_CLIENTCLASS_DT(C_LightOrigin, DT_LightOrigin, CLightOrigin)
RecvPropVector(RECVINFO(LightEnvVector))
END_RECV_TABLE()

void C_LightOrigin::Update()
{
	SetAbsOrigin(C_BasePlayer::GetLocalPlayer()->GetAbsOrigin());
}


void C_LightOrigin::Simulate()
{
	Update();
	BaseClass::Simulate();
}

ConVar bebra("csm_filter", "1");

//-----------------------------------------------------------------------------
// Purpose: main csm code	
//-----------------------------------------------------------------------------

class C_EnvCascadeLight : public C_BaseEntity
{
	DECLARE_CLASS(C_EnvCascadeLight, C_BaseEntity);
public:
	DECLARE_CLIENTCLASS();

	virtual void OnDataChanged(DataUpdateType_t updateType);
	void	ShutDownLightHandle(void);

	virtual void Simulate();

	void	UpdateLight(bool bForceUpdate);

	C_EnvCascadeLight();
	~C_EnvCascadeLight();


private:

	ClientShadowHandle_t m_LightHandle;

	EHANDLE	m_hTargetEntity;

	void updatePos();
	CBaseEntity* pEntity = NULL;
	bool	firstUpdate = true;
	bool	m_bState;
	float	m_flLightFOV;
	bool	m_bEnableShadows;
	bool	m_bLightOnlyTarget;
	bool	m_bLightWorld;
	bool	m_bCameraSpace;
	Vector	m_LinearFloatLightColor;
	float	m_flAmbient;
	float	m_flNearZ;
	float	m_flFarZ;
	char	m_SpotlightTextureName[MAX_PATH];
	int		m_nSpotlightTextureFrame;
	int		m_nShadowQuality;
};

IMPLEMENT_CLIENTCLASS_DT(C_EnvCascadeLight, DT_EnvCascadeLight, CEnvCascadeLight)
	RecvPropEHandle( RECVINFO( m_hTargetEntity )	),
	RecvPropBool(	 RECVINFO( m_bState )			),
	RecvPropFloat(	 RECVINFO( m_flLightFOV )		),
	RecvPropBool(	 RECVINFO( m_bEnableShadows )	),
	RecvPropBool(	 RECVINFO( m_bLightOnlyTarget ) ),
	RecvPropBool(	 RECVINFO( m_bLightWorld )		),
	RecvPropBool(	 RECVINFO( m_bCameraSpace )		),
	RecvPropVector(	 RECVINFO( m_LinearFloatLightColor )		),
	RecvPropFloat(	 RECVINFO( m_flAmbient )		),
	RecvPropString(  RECVINFO( m_SpotlightTextureName ) ),
	RecvPropInt(	 RECVINFO( m_nSpotlightTextureFrame ) ),
	RecvPropFloat(	 RECVINFO( m_flNearZ )	),
	RecvPropFloat(	 RECVINFO( m_flFarZ )	),
	RecvPropInt(	 RECVINFO( m_nShadowQuality )	)
END_RECV_TABLE()

C_EnvCascadeLight::C_EnvCascadeLight( void )
{
	
	m_LightHandle = CLIENTSHADOW_INVALID_HANDLE;
}


C_EnvCascadeLight::~C_EnvCascadeLight( void )
{
	ShutDownLightHandle();
}

void C_EnvCascadeLight::ShutDownLightHandle( void )
{
	// Clear out the light
	if( m_LightHandle != CLIENTSHADOW_INVALID_HANDLE )
	{
		g_pClientShadowMgr->DestroyFlashlight( m_LightHandle );
		m_LightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

void C_EnvCascadeLight::updatePos()
{
	if (firstUpdate)
	{
		m_flNearZ = 4000;
		m_flFarZ = 16000;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - how do you increase the light's height?
//-----------------------------------------------------------------------------
void C_EnvCascadeLight::OnDataChanged( DataUpdateType_t updateType )
{
	UpdateLight( true );
	BaseClass::OnDataChanged( updateType );
}



void C_EnvCascadeLight::UpdateLight( bool bForceUpdate )
{

	if (m_bState == false)
	{
		if (m_LightHandle != CLIENTSHADOW_INVALID_HANDLE)
		{
			ShutDownLightHandle();
		}

		return;
	}

	Vector vForward, vRight, vUp, vPos = GetAbsOrigin();
	FlashlightState_t state;
	state.m_flShadowFilterSize = bebra.GetFloat();
	

	if (m_hTargetEntity != NULL)
	{
		if (m_bCameraSpace)
		{
			const QAngle& angles = GetLocalAngles();

			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
			if (pPlayer)
			{
				const QAngle playerAngles = pPlayer->GetAbsAngles();

				Vector vPlayerForward, vPlayerRight, vPlayerUp;
				AngleVectors(playerAngles, &vPlayerForward, &vPlayerRight, &vPlayerUp);

				matrix3x4_t	mRotMatrix;
				AngleMatrix(angles, mRotMatrix);

				VectorITransform(vPlayerForward, mRotMatrix, vForward);
				VectorITransform(vPlayerRight, mRotMatrix, vRight);
				VectorITransform(vPlayerUp, mRotMatrix, vUp);

				float dist = (m_hTargetEntity->GetAbsOrigin() - GetAbsOrigin()).Length();
				vPos = m_hTargetEntity->GetAbsOrigin() - vForward * dist;

				VectorNormalize(vForward);
				VectorNormalize(vRight);
				VectorNormalize(vUp);
			}
		}
		else
		{
			vForward = m_hTargetEntity->GetAbsOrigin() - GetAbsOrigin();
			VectorNormalize(vForward);

			Assert(0);

		}
	}
	else
	{
		AngleVectors(GetAbsAngles(), &vForward, &vRight, &vUp);
	}


	state.m_fHorizontalFOVDegrees = m_flLightFOV;
	state.m_fVerticalFOVDegrees = m_flLightFOV;

	state.m_vecLightOrigin = vPos;
	BasisToQuaternion(vForward, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = 0.0;
	state.m_fLinearAtten = 100;
	state.m_fConstantAtten = 0.0f;
	state.m_Color[0] = m_LinearFloatLightColor.x;
	state.m_Color[1] = m_LinearFloatLightColor.y;
	state.m_Color[2] = m_LinearFloatLightColor.z;
	state.m_Color[3] = m_flAmbient; // fixme: need to make ambient work m_flAmbient;
	state.m_NearZ = m_flNearZ;
	
	state.m_FarZ = m_flFarZ;
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap.GetFloat();
	state.m_bEnableShadows = m_bEnableShadows;
	state.m_pSpotlightTexture = materials->FindTexture(m_SpotlightTextureName, TEXTURE_GROUP_OTHER, false);
	state.m_nSpotlightTextureFrame = m_nSpotlightTextureFrame;

	state.m_nShadowQuality = m_nShadowQuality; // Allow entity to affect shadow quality
#ifdef MAPBASE

	state.m_bOrtho = csm_ortho.GetBool();
	if (state.m_bOrtho)
	{
		state.m_fOrthoLeft = csm_ortho_left.GetInt();
		state.m_fOrthoTop = csm_ortho_top.GetInt();
		state.m_fOrthoRight = csm_ortho_right.GetInt();
		state.m_fOrthoBottom = csm_ortho_bottom.GetInt();

		state.m_fLinearAtten = ConVarRef("csm_current_distance").GetInt() * 2;
		state.m_FarZAtten = ConVarRef("csm_current_distance").GetInt() * 2;
	}
#endif // MAPBASE

	if (m_LightHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_LightHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (m_hTargetEntity != NULL || bForceUpdate == true)
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_LightHandle, state);
		}
	}

	if (m_bLightOnlyTarget)
	{
		g_pClientShadowMgr->SetFlashlightTarget(m_LightHandle, m_hTargetEntity);
	}
	else
	{
		g_pClientShadowMgr->SetFlashlightTarget(m_LightHandle, NULL);
	}

	g_pClientShadowMgr->SetFlashlightLightWorld(m_LightHandle, m_bLightWorld);


		g_pClientShadowMgr->UpdateProjectedTexture(m_LightHandle, true);

		//mat_slopescaledepthbias_shadowmap.SetValue("4");
		//mat_depthbias_shadowmap.SetValue("0.00001");
		scissor.SetValue("0");

}

void C_EnvCascadeLight::Simulate( void )
{
	UpdateLight( true );
	updatePos();
	firstUpdate = false;
	BaseClass::Simulate();
}


//-----------------------------------------------------------------------------
// Purpose:	second csm	
//-----------------------------------------------------------------------------

class C_EnvCascadeLightSecond : public C_BaseEntity
{
	DECLARE_CLASS(C_EnvCascadeLightSecond, C_BaseEntity);
public:
	DECLARE_CLIENTCLASS();

	virtual void OnDataChanged(DataUpdateType_t updateType);
	void	ShutDownLightHandle(void);

	virtual void Simulate();

	void	UpdateLight(bool bForceUpdate);
	void updatePos();

	C_EnvCascadeLightSecond();
	~C_EnvCascadeLightSecond();


private:

	ClientShadowHandle_t m_LightHandle;

	EHANDLE	m_hTargetEntity;
	CBaseEntity* pEntity = NULL;
	bool	firstUpdate = true;
	bool	m_bState;	
	float	m_flLightFOV;
	bool	m_bEnableShadows;
	bool	m_bLightOnlyTarget;
	bool	m_bLightWorld;
	bool	m_bCameraSpace;
	Vector	m_LinearFloatLightColor;
	float	m_flAmbient;
	float	m_flNearZ;
	float	m_flFarZ;
	char	m_SpotlightTextureName[MAX_PATH];
	int		m_nSpotlightTextureFrame;
	int		m_nShadowQuality;
};

IMPLEMENT_CLIENTCLASS_DT(C_EnvCascadeLightSecond, DT_EnvCascadeLightSecond, CEnvCascadeLightSecond)
RecvPropEHandle(RECVINFO(m_hTargetEntity)),
RecvPropBool(RECVINFO(m_bState)),
RecvPropFloat(RECVINFO(m_flLightFOV)),
RecvPropBool(RECVINFO(m_bEnableShadows)),
RecvPropBool(RECVINFO(m_bLightOnlyTarget)),
RecvPropBool(RECVINFO(m_bLightWorld)),
RecvPropBool(RECVINFO(m_bCameraSpace)),
RecvPropVector(RECVINFO(m_LinearFloatLightColor)),
RecvPropFloat(RECVINFO(m_flAmbient)),
RecvPropString(RECVINFO(m_SpotlightTextureName)),
RecvPropInt(RECVINFO(m_nSpotlightTextureFrame)),
RecvPropFloat(RECVINFO(m_flNearZ)),
RecvPropFloat(RECVINFO(m_flFarZ)),
RecvPropInt(RECVINFO(m_nShadowQuality))
END_RECV_TABLE()

C_EnvCascadeLightSecond::C_EnvCascadeLightSecond(void)
{

	m_LightHandle = CLIENTSHADOW_INVALID_HANDLE;
}


C_EnvCascadeLightSecond::~C_EnvCascadeLightSecond(void)
{
	ShutDownLightHandle();
}

void C_EnvCascadeLightSecond::ShutDownLightHandle(void)
{
	// Clear out the light
	if (m_LightHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_LightHandle);
		m_LightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

void C_EnvCascadeLightSecond::OnDataChanged(DataUpdateType_t updateType)
{
	UpdateLight(true);
	BaseClass::OnDataChanged(updateType);
}

void C_EnvCascadeLightSecond::UpdateLight(bool bForceUpdate)
{

	if (m_bState == false)
	{
		if (m_LightHandle != CLIENTSHADOW_INVALID_HANDLE)
		{
			ShutDownLightHandle();
		}

		return;
	}

	Vector vForward, vRight, vUp, vPos = GetAbsOrigin();
	FlashlightState_t state;
	state.m_flShadowFilterSize = bebra.GetFloat();

	if (m_hTargetEntity != NULL)
	{
		if (m_bCameraSpace)
		{
			const QAngle& angles = GetLocalAngles();

			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
			if (pPlayer)
			{
				const QAngle playerAngles = pPlayer->GetAbsAngles();

				Vector vPlayerForward, vPlayerRight, vPlayerUp;
				AngleVectors(playerAngles, &vPlayerForward, &vPlayerRight, &vPlayerUp);

				matrix3x4_t	mRotMatrix;
				AngleMatrix(angles, mRotMatrix);

				VectorITransform(vPlayerForward, mRotMatrix, vForward);
				VectorITransform(vPlayerRight, mRotMatrix, vRight);
				VectorITransform(vPlayerUp, mRotMatrix, vUp);

				float dist = (m_hTargetEntity->GetAbsOrigin() - GetAbsOrigin()).Length();
				vPos = m_hTargetEntity->GetAbsOrigin() - vForward * dist;

				VectorNormalize(vForward);
				VectorNormalize(vRight);
				VectorNormalize(vUp);
			}
		}
		else
		{
			vForward = m_hTargetEntity->GetAbsOrigin() - GetAbsOrigin();
			VectorNormalize(vForward);
			Assert(0);
		}
	}
	else
	{
		AngleVectors(GetAbsAngles(), &vForward, &vRight, &vUp);
	}


	state.m_fHorizontalFOVDegrees = m_flLightFOV;
	state.m_fVerticalFOVDegrees = m_flLightFOV;

	state.m_vecLightOrigin = vPos;
	BasisToQuaternion(vForward, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = 0.0;
	state.m_fLinearAtten = 100;
	state.m_fConstantAtten = 0.0f;
	state.m_Color[0] = m_LinearFloatLightColor.x;
	state.m_Color[1] = m_LinearFloatLightColor.y;
	state.m_Color[2] = m_LinearFloatLightColor.z;
	state.m_Color[3] = 0.0f; // fixme: need to make ambient work m_flAmbient;
	state.m_NearZ = m_flNearZ;
	state.m_FarZ = m_flFarZ;
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap.GetFloat();
	state.m_bEnableShadows = m_bEnableShadows;
	state.m_pSpotlightTexture = materials->FindTexture(m_SpotlightTextureName, TEXTURE_GROUP_OTHER, false);
	state.m_nSpotlightTextureFrame = m_nSpotlightTextureFrame;

	state.m_nShadowQuality = m_nShadowQuality; // Allow entity to affect shadow quality
	
#ifdef MAPBASE

	state.m_bOrtho = csm_ortho.GetBool();
	if(state.m_bOrtho)
	{
		float flOrthoSize = 1000.0f;

		state.m_fOrthoLeft = -flOrthoSize;
		state.m_fOrthoTop = -flOrthoSize;
		state.m_fOrthoRight = flOrthoSize;
		state.m_fOrthoBottom = flOrthoSize;

		state.m_fLinearAtten = ConVarRef("csm_current_distance").GetInt() * 2;
		state.m_FarZAtten = ConVarRef("csm_current_distance").GetInt() * 2;
	}
	
#endif

	if (m_LightHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_LightHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (m_hTargetEntity != NULL || bForceUpdate == true)
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_LightHandle, state);
		}
	}

	if (m_bLightOnlyTarget)
	{
		g_pClientShadowMgr->SetFlashlightTarget(m_LightHandle, m_hTargetEntity);
	}
	else
	{
		g_pClientShadowMgr->SetFlashlightTarget(m_LightHandle, NULL);
	}

	g_pClientShadowMgr->SetFlashlightLightWorld(m_LightHandle, m_bLightWorld);

#ifdef MAPBASE
	if (state.m_bOrtho)
	{
		bool bSupressWorldLights = false;

		bSupressWorldLights = m_bEnableShadows;

		g_pClientShadowMgr->SetShadowFromWorldLightsEnabled(!bSupressWorldLights);
	}
#endif // MAPBASE

	g_pClientShadowMgr->UpdateProjectedTexture(m_LightHandle, true);

	m_flLightFOV = ConVarRef("csm_second_fov").GetFloat();
	
	//mat_slopescaledepthbias_shadowmap.SetValue("4");
	//mat_depthbias_shadowmap.SetValue("0.00001");
	scissor.SetValue("0");

}


void C_EnvCascadeLightSecond::Simulate(void)
{
	UpdateLight(true);
	BaseClass::Simulate();
}
