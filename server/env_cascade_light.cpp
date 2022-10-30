//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity to control screen overlays on a player
//
//=============================================================================

#include "cbase.h"
#include "shareddefs.h"
#include "lights.h"
#include "tier1/utlstring.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



#define ENV_CASCADE_STARTON			(1<<0)

static ConVar defdist("csm_default_distance", "1000", FCVAR_DEVELOPMENTONLY, "Default Z distance. Used for some fov calculations. Please dont change");
static ConVar curdist("csm_current_distance","14000", 0, "Current Z distance. You can change it.");
static ConVar defFOV("csm_default_fov","15", FCVAR_DEVELOPMENTONLY, "Default FOV. Used for some fov calculations. Please dont change");
static ConVar curFOV("csm_current_fov","15", 0, "Current FOV. You can change it");
static ConVar csm_second_fov("csm_second_fov", "26", FCVAR_NONE ,"FOV of the second csm.");
ConVar csm_enable("csm_enable", "1");
//ConVar csm_ortho("csm_ortho", "0");

class CLightOrigin : public CPointEntity
{
	DECLARE_CLASS(CLightOrigin, CPointEntity);
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	void Spawn();

	CLightOrigin();

	bool angFEnv = false;
	
	void InitialThink(void);

private:

	CNetworkVector(LightEnvVector);
};

LINK_ENTITY_TO_CLASS(csmorigin, CLightOrigin);

BEGIN_DATADESC(CLightOrigin)
DEFINE_FIELD(LightEnvVector, FIELD_VECTOR),
DEFINE_THINKFUNC(InitialThink)
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CLightOrigin, DT_LightOrigin)
SendPropVector(SENDINFO(LightEnvVector))
END_SEND_TABLE()

CLightOrigin::CLightOrigin() 
{
}

void CLightOrigin::Spawn()
{
	if (angFEnv)
	{
		CBaseEntity* pEntity = NULL;
		pEntity = gEntList.FindEntityByClassname(pEntity, "light_environment");
		if (pEntity)
		{
			CEnvLight* pEnv = dynamic_cast<CEnvLight*>(pEntity);

			QAngle bb = pEnv->GetAbsAngles();
			//bb.x = bb.x;
			SetAbsAngles(bb);
			
			ConColorMsg(Color(0,230,0), "light_environment Founded!\n");
		}
		else
		{
			//Msg("What the fuck? Map dont have light_environment with targetname!");
			ConColorMsg(Color(230, 0, 0), "What the fuck? Map dont have light_environment with targetname!\n");
		}
	}
}

void CLightOrigin::InitialThink()
{
}


//-----------------------------------------------------------------------------
// Purpose: second csm
//-----------------------------------------------------------------------------

class CEnvCascadeLightSecond : public CPointEntity
{
	DECLARE_CLASS(CEnvCascadeLightSecond, CPointEntity);
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CEnvCascadeLightSecond();
	bool KeyValue(const char* szKeyName, const char* szValue);

	// Always transmit to clients
	virtual int UpdateTransmitState();
	virtual void Activate(void);

	void InitialThink(void);

	CNetworkHandle(CBaseEntity, m_hTargetEntity);
	CNetworkVector(m_LinearFloatLightColor);
	CNetworkColor32(m_LightColor);

private:
	
	CNetworkVar(bool, m_bState);
	CNetworkVar(float, m_flLightFOV);
	CNetworkVar(bool, m_bEnableShadows);
	CNetworkVar(bool, m_bLightOnlyTarget);
	CNetworkVar(bool, m_bLightWorld);
	CNetworkVar(bool, m_bCameraSpace);
	CNetworkVar(float, m_flAmbient);
	CNetworkString(m_SpotlightTextureName, MAX_PATH);
	CNetworkVar(int, m_nSpotlightTextureFrame);
	CNetworkVar(float, m_flNearZ);
	CNetworkVar(float, m_flFarZ);
	CNetworkVar(int, m_nShadowQuality);
};

LINK_ENTITY_TO_CLASS(second_csm, CEnvCascadeLightSecond);

BEGIN_DATADESC(CEnvCascadeLightSecond)
DEFINE_FIELD(m_hTargetEntity, FIELD_EHANDLE),
DEFINE_FIELD(m_bState, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_flLightFOV, FIELD_FLOAT, "lightfov"),
DEFINE_KEYFIELD(m_bEnableShadows, FIELD_BOOLEAN, "enableshadows"),
DEFINE_KEYFIELD(m_bLightOnlyTarget, FIELD_BOOLEAN, "lightonlytarget"),
DEFINE_KEYFIELD(m_bLightWorld, FIELD_BOOLEAN, "lightworld"),
DEFINE_KEYFIELD(m_bCameraSpace, FIELD_BOOLEAN, "cameraspace"),
DEFINE_KEYFIELD(m_flAmbient, FIELD_FLOAT, "ambient"),
DEFINE_AUTO_ARRAY_KEYFIELD(m_SpotlightTextureName, FIELD_CHARACTER, "texturename"),
DEFINE_KEYFIELD(m_nSpotlightTextureFrame, FIELD_INTEGER, "textureframe"),
DEFINE_KEYFIELD(m_flNearZ, FIELD_FLOAT, "nearz"),
DEFINE_KEYFIELD(m_flFarZ, FIELD_FLOAT, "farz"),
DEFINE_KEYFIELD(m_nShadowQuality, FIELD_INTEGER, "shadowquality"),
DEFINE_FIELD(m_LinearFloatLightColor, FIELD_VECTOR),
DEFINE_THINKFUNC(InitialThink),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CEnvCascadeLightSecond, DT_EnvCascadeLightSecond)
SendPropInt(SENDINFO(m_LightColor), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt),
SendPropEHandle(SENDINFO(m_hTargetEntity)),
SendPropBool(SENDINFO(m_bState)),
SendPropFloat(SENDINFO(m_flLightFOV)),
SendPropBool(SENDINFO(m_bEnableShadows)),
SendPropBool(SENDINFO(m_bLightOnlyTarget)),
SendPropBool(SENDINFO(m_bLightWorld)),
SendPropBool(SENDINFO(m_bCameraSpace)),
SendPropVector(SENDINFO(m_LinearFloatLightColor)),
SendPropFloat(SENDINFO(m_flAmbient)),
SendPropString(SENDINFO(m_SpotlightTextureName)),
SendPropInt(SENDINFO(m_nSpotlightTextureFrame)),
SendPropFloat(SENDINFO(m_flNearZ), 16, SPROP_ROUNDDOWN, 0.0f, 500.0f),
SendPropFloat(SENDINFO(m_flFarZ), 18, SPROP_ROUNDDOWN, 0.0f, 1500.0f),
SendPropInt(SENDINFO(m_nShadowQuality), 1, SPROP_UNSIGNED)  // Just one bit for now
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEnvCascadeLightSecond::CEnvCascadeLightSecond(void)
{
#ifdef MAPBASE
	m_LightColor.Init(255, 255, 255, 255);
#else
	m_LightColor.Init(255, 255, 255, 1);
#endif
	m_bState = csm_enable.GetBool();
	m_flLightFOV = 45.0f;
	m_bEnableShadows = true;
	m_bLightOnlyTarget = false;
	m_bLightWorld = true;
	m_bCameraSpace = false;

	Q_strcpy(m_SpotlightTextureName.GetForModify(), "tools\\fakecsm\\mask_ring");
	m_nSpotlightTextureFrame = 0;
	m_LinearFloatLightColor.Init(1.0f, 1.0f, 1.0f);
	m_flAmbient = 0.0f;
	m_flNearZ = 8000.0f;
	m_flFarZ = 16000.0f;
	m_nShadowQuality = 0;
}

void UTIL_ColorStringToLinearFloatColorCSMFakeSecond(Vector& color, const char* pString)
{
	float tmp[4];
	UTIL_StringToFloatArray(tmp, 4, pString);
	if (tmp[3] <= 0.0f)
	{
		tmp[3] = 255.0f;
	}
	tmp[3] *= (1.0f / 255.0f);
	color.x = GammaToLinear(tmp[0] * (1.0f / 255.0f)) * tmp[3];
	color.y = GammaToLinear(tmp[1] * (1.0f / 255.0f)) * tmp[3];
	color.z = GammaToLinear(tmp[2] * (1.0f / 255.0f)) * tmp[3];
}

bool CEnvCascadeLightSecond::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "lightcolor"))
	{
		Vector tmp;
		UTIL_ColorStringToLinearFloatColorCSMFakeSecond(tmp, szValue);
		m_LinearFloatLightColor = tmp;
	}
	else
	{
		return BaseClass::KeyValue(szKeyName, szValue);
	}

	return true;
}

void CEnvCascadeLightSecond::Activate(void)
{
	if (GetSpawnFlags() & ENV_CASCADE_STARTON)
	{
		m_bState = true;
	}

	SetThink(&CEnvCascadeLightSecond::InitialThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	BaseClass::Activate();
}

void CEnvCascadeLightSecond::InitialThink(void)
{
	m_bState = csm_enable.GetBool();
	float bibigon = defdist.GetFloat() / curdist.GetFloat();
	m_flLightFOV = csm_second_fov.GetFloat() * bibigon;
	m_hTargetEntity = gEntList.FindEntityByName(NULL, m_target);
}

int CEnvCascadeLightSecond::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}



//-----------------------------------------------------------------------------
// Purpose: main csm code
//-----------------------------------------------------------------------------
class CEnvCascadeLight : public CPointEntity
{
	DECLARE_CLASS(CEnvCascadeLight, CPointEntity);
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CEnvCascadeLight();
	bool KeyValue(const char* szKeyName, const char* szValue);

	// Always transmit to clients
	virtual int UpdateTransmitState();
	virtual void Activate(void);
	void Spawn();
	void Preparation();

	//Inputs
	void InputTurnOn(inputdata_t& inputdata);
	void InputTurnOff(inputdata_t& inputdata);
	void InputSetEnableShadows(inputdata_t& inputdata);
	void InputSetLightColor( inputdata_t &inputdata );
	void InputSetSpotlightTexture(inputdata_t& inputdata);
	void InputSetAmbient(inputdata_t& inputdata);
	void InputSetAngles(inputdata_t& inputdata);
	void InputAddAngles(inputdata_t& inputdata);
	void InputResetAngles(inputdata_t& inputdata);

	void InitialThink(void);



	CNetworkHandle(CBaseEntity, m_hTargetEntity);

private:
	CNetworkColor32(m_LightColor);
	CLightOrigin* csm_origin;
	CEnvCascadeLightSecond* SecondCSM;
	CNetworkVar(bool, m_bState);
	CNetworkVar(float, m_flLightFOV);
	CNetworkVar(bool, EnableAngleFromEnv);
	CNetworkVar(bool, m_bEnableShadows);
	CNetworkVar(bool, m_bLightOnlyTarget);
	CNetworkVar(bool, m_bLightWorld);
	CNetworkVar(bool, m_bCameraSpace);
	CNetworkVector(m_LinearFloatLightColor);
	CNetworkVar(float, m_flAmbient);
	CNetworkString(m_SpotlightTextureName, MAX_PATH);
	CNetworkVar(int, m_nSpotlightTextureFrame);
	CNetworkVar(float, m_flNearZ);
	CNetworkVar(float, m_flFarZ);
	CNetworkVar(int, m_nShadowQuality);

	QAngle DefaultAngle = QAngle(0, 0, 0);
	QAngle CurrentAngle = QAngle(0, 0, 0);
};

LINK_ENTITY_TO_CLASS(env_cascade_light, CEnvCascadeLight);

BEGIN_DATADESC(CEnvCascadeLight)
DEFINE_FIELD(m_hTargetEntity, FIELD_EHANDLE),
DEFINE_FIELD(m_bState, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_bEnableShadows, FIELD_BOOLEAN, "enableshadows"),
DEFINE_KEYFIELD(m_bLightOnlyTarget, FIELD_BOOLEAN, "lightonlytarget"),
DEFINE_KEYFIELD(m_bLightWorld, FIELD_BOOLEAN, "lightworld"),
DEFINE_KEYFIELD(m_bCameraSpace, FIELD_BOOLEAN, "cameraspace"),
DEFINE_KEYFIELD(m_flAmbient, FIELD_FLOAT, "ambient"),
DEFINE_AUTO_ARRAY_KEYFIELD(m_SpotlightTextureName, FIELD_CHARACTER, "texturename"),
DEFINE_KEYFIELD(m_nSpotlightTextureFrame, FIELD_INTEGER, "textureframe"),
DEFINE_KEYFIELD(m_flNearZ, FIELD_FLOAT, "nearz"),
DEFINE_KEYFIELD(m_flFarZ, FIELD_FLOAT, "farz"),
DEFINE_KEYFIELD(m_nShadowQuality, FIELD_INTEGER, "shadowquality"),
DEFINE_FIELD(m_LinearFloatLightColor, FIELD_VECTOR),
DEFINE_KEYFIELD(EnableAngleFromEnv, FIELD_BOOLEAN, "uselightenvangles"),


//Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputTurnOn),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputTurnOff),
DEFINE_INPUTFUNC(FIELD_BOOLEAN, "EnableShadows", InputSetEnableShadows),
DEFINE_INPUTFUNC(FIELD_COLOR32, "LightColor", InputSetLightColor),
DEFINE_INPUTFUNC(FIELD_FLOAT, "Ambient", InputSetAmbient),
DEFINE_INPUTFUNC(FIELD_STRING, "Texture", InputSetSpotlightTexture),
DEFINE_INPUTFUNC(FIELD_STRING, "SetAngles", InputSetAngles),
DEFINE_INPUTFUNC(FIELD_STRING, "AddAngles", InputAddAngles),
DEFINE_INPUTFUNC(FIELD_VOID, "ResetAngles", InputResetAngles),

DEFINE_THINKFUNC(InitialThink),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CEnvCascadeLight, DT_EnvCascadeLight)

	SendPropInt(SENDINFO(m_LightColor), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt),

	SendPropEHandle(SENDINFO(m_hTargetEntity)),
	SendPropBool(SENDINFO(m_bState)),
	SendPropFloat(SENDINFO(m_flLightFOV)),
	SendPropBool(SENDINFO(m_bEnableShadows)),
	SendPropBool(SENDINFO(m_bLightOnlyTarget)),
	SendPropBool(SENDINFO(m_bLightWorld)),
	SendPropBool(SENDINFO(m_bCameraSpace)),
	SendPropVector(SENDINFO(m_LinearFloatLightColor)),
	SendPropFloat(SENDINFO(m_flAmbient)),
	SendPropString(SENDINFO(m_SpotlightTextureName)),
	SendPropInt(SENDINFO(m_nSpotlightTextureFrame)),
	SendPropFloat(SENDINFO(m_flNearZ), 16, SPROP_ROUNDDOWN, 0.0f, 500.0f),
	SendPropFloat(SENDINFO(m_flFarZ), 18, SPROP_ROUNDDOWN, 0.0f, 1500.0f),
	SendPropInt(SENDINFO(m_nShadowQuality), 1, SPROP_UNSIGNED)  // Just one bit for now
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEnvCascadeLight::CEnvCascadeLight(void)
{
#ifdef MAPBASE
	m_LightColor.Init(255, 255, 255, 255);
#else
	m_LightColor.Init(255, 255, 255, 1);
#endif
	m_bState = csm_enable.GetBool();
	m_flLightFOV = 45.0f;
	m_bEnableShadows = true;
	m_bLightOnlyTarget = false;
	m_bLightWorld = true;
	m_bCameraSpace = false;
	EnableAngleFromEnv = false;

	Q_strcpy(m_SpotlightTextureName.GetForModify(), "tools\\fakecsm\\mask_center");
	m_nSpotlightTextureFrame = 0;
	m_LinearFloatLightColor.Init(1.0f, 1.0f, 1.0f);
	m_flAmbient = 0.0f;
	m_flNearZ = 8000.0f;
	m_flFarZ = 16000.0f;
	m_nShadowQuality = 0;

	
}
ConVar csm_second_intensity("csm_second_intensity", "2");

void CEnvCascadeLight::Preparation()
{
	CreateEntityByName("csmorigin");
	CreateEntityByName("second_csm");
	defFOV.SetValue(m_flLightFOV);
	CBaseEntity* CSMOrigin = NULL;
	CBaseEntity* CSMSecond = NULL;

	CSMOrigin = gEntList.FindEntityByClassname(CSMOrigin, "csmorigin");
	CSMSecond = gEntList.FindEntityByClassname(CSMSecond, "second_csm");
	//if origin is exist
	if (CSMOrigin)
	{

		csm_origin = dynamic_cast<CLightOrigin*>(CSMOrigin);
		//if second csm is exist

		if (CSMSecond)
		{

			SecondCSM = dynamic_cast<CEnvCascadeLightSecond*>(CSMSecond);
			SecondCSM->SetAbsAngles(GetAbsAngles());
			SecondCSM->SetAbsOrigin(GetAbsOrigin());
			SecondCSM->SetParent(GetBaseEntity());
			SecondCSM->m_LinearFloatLightColor = m_LinearFloatLightColor * csm_second_intensity.GetFloat();

			DispatchSpawn(SecondCSM);
		}


		SetParent(csm_origin, 1);

		SetAbsOrigin(Vector(csm_origin->GetAbsOrigin().x, csm_origin->GetAbsOrigin().y, csm_origin->GetAbsOrigin().z + curdist.GetInt()));

		if (EnableAngleFromEnv)
		{


			csm_origin->angFEnv = true;
			SetLocalAngles(QAngle(90, 0, 0));

		}
		else
		{
			csm_origin->SetAbsAngles(QAngle((GetLocalAngles().x - 90), GetLocalAngles().y, -GetLocalAngles().z));
			//Msg("pEnv local angle = %f %f %f \n", pEnv->GetLocalAngles().x, pEnv->GetLocalAngles().y, pEnv->GetLocalAngles().z);
			SetLocalAngles(QAngle(90, 0, 0));
			DevMsg("CSM using light_environment \n");
		}



		DefaultAngle = csm_origin->GetAbsAngles();
		CurrentAngle = DefaultAngle;


		DispatchSpawn(CSMOrigin);
	}
	else
	{
		Msg("Main csm entity can't find \"csmorigin\" entity!");
	}

}

void CEnvCascadeLight::Spawn()
{
	
	Preparation();

}

void UTIL_ColorStringToLinearFloatColorCSMFake(Vector& color, const char* pString)
{
	float tmp[4];
	UTIL_StringToFloatArray(tmp, 4, pString);
	if (tmp[3] <= 0.0f)
	{
		tmp[3] = 255.0f;
	}
	tmp[3] *= (1.0f / 255.0f);
	color.x = GammaToLinear(tmp[0] * (1.0f / 255.0f)) * tmp[3];
	color.y = GammaToLinear(tmp[1] * (1.0f / 255.0f)) * tmp[3];
	color.z = GammaToLinear(tmp[2] * (1.0f / 255.0f)) * tmp[3];
}

bool CEnvCascadeLight::KeyValue(const char* szKeyName, const char* szValue)
{

	if (FStrEq(szKeyName, "lightcolor") || FStrEq(szKeyName, "color"))

	{
		Vector tmp;
		UTIL_ColorStringToLinearFloatColorCSMFake(tmp, szValue);
		m_LinearFloatLightColor = tmp;
	}
	else
	{
		return BaseClass::KeyValue(szKeyName, szValue);
	}

	return true;
}

void CEnvCascadeLight::InputTurnOn(inputdata_t& inputdata)
{
	m_bState = true;
}

void CEnvCascadeLight::InputTurnOff(inputdata_t& inputdata)
{
	m_bState = false;
}


void CEnvCascadeLight::InputSetEnableShadows(inputdata_t& inputdata)
{
	m_bEnableShadows = inputdata.value.Bool();
}


void CEnvCascadeLight::InputSetAmbient(inputdata_t& inputdata)
{
	m_flAmbient = inputdata.value.Float();
}

void CEnvCascadeLight::InputSetSpotlightTexture(inputdata_t& inputdata)
{
	Q_strcpy(m_SpotlightTextureName.GetForModify(), inputdata.value.String());
}

void CEnvCascadeLight::Activate(void)
{
	if (GetSpawnFlags() & ENV_CASCADE_STARTON)
	{
		m_bState = true;
	}
	SetThink(&CEnvCascadeLight::InitialThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	BaseClass::Activate();
}

void CEnvCascadeLight::InitialThink(void)
{
	m_bState = csm_enable.GetBool();
	m_hTargetEntity = gEntList.FindEntityByName(NULL, m_target);
	float bibigon = defdist.GetFloat() / curdist.GetFloat();
	curFOV.SetValue(defFOV.GetFloat() * bibigon);
	m_flLightFOV = curFOV.GetFloat();
}


void CEnvCascadeLight::InputSetAngles(inputdata_t& inputdata)
{
	const char* pAngles = inputdata.value.String();

	QAngle angles;
	UTIL_StringToVector(angles.Base(), pAngles); 
	
	CurrentAngle = angles;
	csm_origin->SetAbsAngles(CurrentAngle);

}

void CEnvCascadeLight::InputAddAngles(inputdata_t& inputdata)
{
	const char* pAngles = inputdata.value.String();

	QAngle angles;
	UTIL_StringToVector(angles.Base(), pAngles);

	CurrentAngle = CurrentAngle + angles;
	csm_origin->SetAbsAngles(CurrentAngle);
}

void CEnvCascadeLight::InputResetAngles(inputdata_t& inputdata)
{
	CurrentAngle = DefaultAngle;
	csm_origin->SetAbsAngles(CurrentAngle);
}

void CEnvCascadeLight::InputSetLightColor(inputdata_t& inputdata)
{
	m_LightColor = inputdata.value.Color32();
	SecondCSM->m_LightColor = inputdata.value.Color32();
	//m_LinearFloatLightColor.Init(1.0f, 1.0f, 1.0f);
}

int CEnvCascadeLight::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}
