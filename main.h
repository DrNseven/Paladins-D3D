#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <time.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#include "DXSDK\d3dx9.h"
#if defined _M_X64
#pragma comment(lib, "DXSDK/x64/d3dx9.lib") 
#elif defined _M_IX86
#pragma comment(lib, "DXSDK/x86/d3dx9.lib")
#endif

//#include <d3dx9.h>
//#pragma comment(lib, "d3dx9.lib") 
#pragma comment(lib, "winmm.lib")
#include "MinHook/include/MinHook.h" //detour
using namespace std;

#pragma warning (disable: 4244) //

//==========================================================================================================================

HMODULE WinHand;

//Stride
UINT Stride;

//elementcount
D3DVERTEXELEMENT9 decl[MAXD3DDECLLENGTH];
UINT numElements;

//vertexshaderconstantf
UINT mStartRegister;
UINT mVector4fCount;

//vdesc.Size
//D3DVERTEXBUFFER_DESC vdesc;

//vertexshader
IDirect3DVertexShader9* vShader;
UINT vSize;

//pixelshader
IDirect3DPixelShader9* pShader;
UINT pSize;

//texture crc
IDirect3DTexture9* pCurrentTex = NULL;
DWORD texCRC;
int dWidth;
int dHeight;
int dFormat;
int mStage;

D3DVIEWPORT9 Viewport; //use this viewport
float ScreenCX;
float ScreenCY;

//logger
//bool logger = false;
//int countnum = -1;

//features
int wallhack = 1;				//wallhack
int occlusion = 1;				//occlusion exploit

//aimbot settings
int aimbot = 2;
int aimkey = 2;
DWORD Daimkey = VK_RBUTTON;		//aimkey
int aimsens = 3;				//aim sensitivity, makes aim smoother
int aimfov = 6;					//aim field of view in % 
int aimheight = 2;				//aim height value for mensa, low value = aims heigher, high values aims lower
int aimheightxy = 1;			//real value, aimheight * 4 + 27
int esp = 0;					//anim pic esp
int aimpause = 0;
bool pause = false;
bool botpause = false;
unsigned int aimpausexy = 1;	//use x-999 (wait for xx millisecs, looks more legit)
bool sound = false;
int killsounds = 0;

//autoshoot settings
int autoshoot = 1;
unsigned int asdelay = 50;		//use x-999 (shoot for xx millisecs, looks more legit)
bool IsPressd = false;			//

//timer
DWORD astime = timeGetTime();	//autoshoot
DWORD framepause = timeGetTime(); //aimpause
DWORD soundpause = timeGetTime(); //killsounds

//sprite timer
DWORD dwStartTime0 = 0; //time as the timer started
DWORD dwTime0 = 0; //windowsuptime
DWORD dwStartTime1 = 0; //time as the timer started
DWORD dwTime1 = 0; //windowsuptime
//==========================================================================================================================

// getdir & log
char windir[320];
char* GetFolderFile(char *fname)
{
	static char pfad[320];
	strcpy_s(pfad, windir);
	strcat_s(pfad, fname);
	return pfad;
}
/*
void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetFolderFile("log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}
*/
DWORD QuickChecks(DWORD *pData, int size)
{
	if (!pData) { return 0x0; }

	DWORD sum;
	DWORD tmp;
	sum = *pData;

	for (int i = 1; i < (size / 4); i++)
	{
		tmp = pData[i];
		tmp = (DWORD)(sum >> 29) + tmp;
		tmp = (DWORD)(sum >> 17) + tmp;
		sum = (DWORD)(sum << 3) ^ tmp;
	}

	return sum;
}

//==========================================================================================================================

void mmousemove(int myX, int myY)
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dx = myX;
	Input.mi.dy = myY;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE;
	SendInput(1, &Input, sizeof(INPUT)); // Move Mouse
}

void LLeftClickDown()
{
	INPUT    Input = { 0 };
	// left down 
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &Input, sizeof(INPUT));
}

void LLeftClickUp()
{
	INPUT    Input = { 0 };
	//::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &Input, sizeof(INPUT));
}

//get distance
float GetDst(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

struct AimHPBarInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDst;
};
std::vector<AimHPBarInfo_t>AimHPBarInfo;
//float RealDistance;

// Parameters:
//
//   float4 mvp[4];
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   mvp          c8       4
//w2s for health bars
void HPBarAim(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	D3DXVECTOR3 from, to;
	D3DXMATRIX mvp, world;

	//not here
	//D3DVIEWPORT9 Viewport;
	//Device->GetViewport(&Viewport);

	Device->GetVertexShaderConstantF(8, mvp, 4);//8mvp
	//Device->GetVertexShaderConstantF(240, world, 4);//world 240-243, 247-252

	float w = 0.0f;
	to[0] = mvp[0] * world._14 + mvp[1] * world._24 + mvp[2] * world._34 + mvp[3];
	to[1] = mvp[4] * world._14 + mvp[5] * world._24 + mvp[6] * world._34 + mvp[7];
	w = mvp[12] * world._14 + mvp[13] * world._24 + mvp[14] * world._34 + mvp[15];

	if (w > 0.01f)
	{
		//aimheightxy = (Viewport.Height / 50) + (aimheight * 3);
		aimheightxy = ((float)Viewport.Height * 0.02f) + (aimheight * 3);

		float invw = 1.0f / w;
		to[0] *= invw;
		to[1] *= invw;

		float x = Viewport.Width / 2.0f;
		float y = Viewport.Height / 2.0f;

		x += 0.5f * to[0] * Viewport.Width + 0.5f;
		y -= 0.5f * to[1] * Viewport.Height - aimheightxy;
		to[0] = x + Viewport.X;
		to[1] = y + Viewport.Y;
	}
	else
	{
		to[0] = -1.0f;
		to[1] = -1.0f;
	}

	//to[0] = X
	//to[1] = Y

		//AimHPBarInfo_t pAimHPBarInfo = { static_cast<float>(to[0] + (Viewport.Width*0.5f)), static_cast<float>(to[1] + (Viewport.Height*0.5f)), iTeam };
		AimHPBarInfo_t pAimHPBarInfo = { static_cast<float>(to[0]), static_cast<float>(to[1]), iTeam };
		AimHPBarInfo.push_back(pAimHPBarInfo);
}

struct EspHPBarInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDst;
};
std::vector<EspHPBarInfo_t>EspHPBarInfo;
//float RealDistance;

// Parameters:
//
//   float4 mvp[4];
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   mvp          c8       4
//w2s for health bars
void HPBarEsp(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	D3DXVECTOR3 from, to;
	D3DXMATRIX mvp, world;

	//not here
	//D3DVIEWPORT9 Viewport;
	//Device->GetViewport(&Viewport);

	Device->GetVertexShaderConstantF(8, mvp, 4);//8mvp
	Device->GetVertexShaderConstantF(240, world, 4);//world 240-243, 247-252

	float w = 0.0f;
	to[0] = mvp[0] * world._14 + mvp[1] * world._24 + mvp[2] * world._34 + mvp[3];
	to[1] = mvp[4] * world._14 + mvp[5] * world._24 + mvp[6] * world._34 + mvp[7];
	//screen.z = worldToScreen.m[2][0] * point.x + worldToScreen.m[2][1] * point.y + worldToScreen.m[2][2] * point.z + worldToScreen.m[2][3];
	w = mvp[12] * world._14 + mvp[13] * world._24 + mvp[14] * world._34 + mvp[15];
	//screen.z /= w;

	if (w > 0.01f)
	{
		//aimheightxy = (Viewport.Height / 50) + (aimheight * 3);
		aimheightxy = ((float)Viewport.Height * 0.02f) + (aimheight * 3);

		float invw = 1.0f / w;
		to[0] *= invw;
		to[1] *= invw;

		float x = Viewport.Width / 2.0f;
		float y = Viewport.Height / 2.0f;

		x += 0.5f * to[0] * Viewport.Width + 0.5f;
		y -= 0.5f * to[1] * Viewport.Height - aimheightxy;
		to[0] = x + Viewport.X;
		to[1] = y + Viewport.Y;
	}
	else
	{
		to[0] = -1.0f;
		to[1] = -1.0f;
	}

	//to[0] = X
	//to[1] = Y

	//EspHPBarInfo_t pEspHPBarInfo = { static_cast<float>(to[0] + (Viewport.Width*0.5f)), static_cast<float>(to[1] + (Viewport.Height*0.5f)), iTeam };
	EspHPBarInfo_t pEspHPBarInfo = { static_cast<float>(to[0]), static_cast<float>(to[1]), iTeam };
	EspHPBarInfo.push_back(pEspHPBarInfo);

	/*
	//esp
	aimheightxy = 47 + (aimheight*3.0f);

	//Device->GetViewport(&Viewport);
	D3DXMATRIX pProjection, pView, pWorld;
	D3DXVECTOR3 vOut(0, 0, 0), vIn(0, 0, aimheightxy);

	Device->GetVertexShaderConstantF(0, pProjection, 4);
	Device->GetVertexShaderConstantF(231, pView, 4);//231
	//Device->GetVertexShaderConstantF(countnum, pWorld, 3);

	D3DXMatrixIdentity(&pWorld);

	//D3DXVECTOR3 VectorMiddle(0, 0, 0), ScreenMiddlee(Viewport.Width / 2.0f, Viewport.Height / 2.0f, 0);
	//D3DXVec3Unproject(&VectorMiddle, &ScreenMiddlee, &Viewport, &pProjection, &pView, &pWorld);

	D3DXVec3Project(&vOut, &vIn, &Viewport, &pProjection, &pView, &pWorld);

	//if (vOut.z < 1.0f)
	//{
	//float RealDistance = pProjection._44; //GetDistance(VectorMiddle.x, VectorMiddle.y, vIn.x, vIn.y);

	//AimHPBarInfo_t pAimHPBarInfo = { static_cast<float>(vOut.x + (Viewport.Width*0.5f)), static_cast<float>(vOut.y + (Viewport.Height*0.5f)), iTeam };//pSize 164 (outline)
	AimHPBarInfo_t pAimHPBarInfo = { static_cast<float>(vOut.x), static_cast<float>(vOut.y), iTeam }; //pSize 136 (glow)
	AimHPBarInfo.push_back(pAimHPBarInfo);
	*/
}

//==========================================================================================================================

struct AimTBarInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDst;
};
std::vector<AimTBarInfo_t>AimTBarInfo;

void TBarAim(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	D3DXVECTOR3 from, to;
	D3DXMATRIX mvp, world;

	Device->GetVertexShaderConstantF(6, mvp, 4);//mvp

	float w = 0.0f;
	to[0] = mvp[0] * world._14 + mvp[1] * world._24 + mvp[2] * world._34 + mvp[3];
	to[1] = mvp[4] * world._14 + mvp[5] * world._24 + mvp[6] * world._34 + mvp[7];
	w = mvp[12] * world._14 + mvp[13] * world._24 + mvp[14] * world._34 + mvp[15];

	if (w > 0.01f)
	{
		//aimheightxy = (Viewport.Height / 50) + (aimheight * 5);
		aimheightxy = ((float)Viewport.Height * 0.02f) + (aimheight * 5);

		float invw = 1.0f / w;
		to[0] *= invw;
		to[1] *= invw;

		float x = Viewport.Width / 2.0f;
		float y = Viewport.Height / 2.0f;

		x += 0.5f * to[0] * Viewport.Width + 0.5f;
		y -= 0.5f * to[1] * Viewport.Height - aimheightxy;
		to[0] = x + Viewport.X;
		to[1] = y + Viewport.Y;
	}
	else
	{
		to[0] = -1.0f;
		to[1] = -1.0f;
	}

	//to[0] = X
	//to[1] = Y

	AimTBarInfo_t pAimTBarInfo = { static_cast<float>(to[0]), static_cast<float>(to[1]), iTeam };
	AimTBarInfo.push_back(pAimTBarInfo);

	/*
	float xx, yy;
	D3DXMATRIX matrix, m1;
	D3DXVECTOR4 position, input;
	Device->GetVertexShaderConstantF(6, m1, 4);//6

	D3DXMatrixTranspose(&matrix, &m1);
	//D3DXVec4Transform(&position, &input, &matrix);

	position.x = input.x * matrix._11 + input.y * matrix._21 + input.z * matrix._31 + matrix._41;
	position.y = input.x * matrix._12 + input.y * matrix._22 + input.z * matrix._32 + matrix._42;
	position.z = input.x * matrix._13 + input.y * matrix._23 + input.z * matrix._33 + matrix._43;
	position.w = input.x * matrix._14 + input.y * matrix._24 + input.z * matrix._34 + matrix._44;

	xx = ((position.x / position.w) * (Viewport.Width / 2.0f)) + Viewport.X + (Viewport.Width / 2.0f);
	yy = Viewport.Y + (Viewport.Height / 1.8f) - ((position.y / position.w) * (Viewport.Height / 1.8f));

	AimTBarInfo_t pAimTBarInfo = { static_cast<float>(xx), static_cast<float>(yy), iTeam };
	AimTBarInfo.push_back(pAimTBarInfo);
	*/
}

//==========================================================================================================================

//-----------------------------------------------------------------------------
// Name: Save()
// Desc: Saves mensa Item states for later Restoration
//-----------------------------------------------------------------------------
/*
void Save(char* szSection, char* szKey, int iValue, LPCSTR file)
{
char szValue[255];
sprintf_s(szValue, "%d", iValue);
WritePrivateProfileString(szSection, szKey, szValue, file);
}
*/
//-----------------------------------------------------------------------------
// Name: Load()
// Desc: Loads mensa Item States From Previously Saved File
//-----------------------------------------------------------------------------
/*
int Load(char* szSection, char* szKey, int iDefaultValue, LPCSTR file)
{
int iResult = GetPrivateProfileInt(szSection, szKey, iDefaultValue, file);
return iResult;
}
*/

#include <string>
#include <fstream>
void SaveCfg()
{
	ofstream fout;
	fout.open(GetFolderFile("palaconfig.ini"), ios::trunc);
	fout << "Wallhack " << wallhack << endl;
	fout << "Occlusion " << occlusion << endl;
	fout << "Esp " << esp << endl;
	fout << "Aimbot " << aimbot << endl;
	fout << "Aimkey " << aimkey << endl;
	fout << "Aimsens " << aimsens << endl;
	fout << "Aimfov " << aimfov << endl;
	fout << "Aimheight " << aimheight << endl;
	fout << "AimdelayAfterKill " << aimpause << endl;
	fout << "Autoshoot " << autoshoot << endl;
	fout << "Killsounds " << killsounds << endl;
	fout.close();
}

void LoadCfg()
{
	ifstream fin;
	string Word = "";
	fin.open(GetFolderFile("palaconfig.ini"), ifstream::in);
	fin >> Word >> wallhack;
	fin >> Word >> occlusion;
	fin >> Word >> esp;
	fin >> Word >> aimbot;
	fin >> Word >> aimkey;
	fin >> Word >> aimsens;
	fin >> Word >> aimfov;
	fin >> Word >> aimheight;
	fin >> Word >> aimpause;
	fin >> Word >> autoshoot;
	fin >> Word >> killsounds;
	fin.close();
}

//==========================================================================================================================
/*
// colors
#define Green				D3DCOLOR_ARGB(255, 000, 255, 000)
#define Red					D3DCOLOR_ARGB(255, 255, 000, 000)
#define Blue				D3DCOLOR_ARGB(255, 000, 000, 255)
#define Orange				D3DCOLOR_ARGB(255, 255, 165, 000)
#define Yellow				D3DCOLOR_ARGB(255, 255, 255, 000)
#define Pink				D3DCOLOR_ARGB(255, 255, 192, 203)
#define Cyan				D3DCOLOR_ARGB(255, 000, 255, 255)
#define Purple				D3DCOLOR_ARGB(255, 160, 032, 240)
#define Black				D3DCOLOR_ARGB(255, 000, 000, 000) 
#define White				D3DCOLOR_ARGB(255, 255, 255, 255)
#define Grey				D3DCOLOR_ARGB(255, 112, 112, 112)
#define SteelBlue			D3DCOLOR_ARGB(255, 033, 104, 140)
#define LightSteelBlue		D3DCOLOR_ARGB(255, 201, 255, 255)
#define LightBlue			D3DCOLOR_ARGB(255, 026, 140, 306)
#define Salmon				D3DCOLOR_ARGB(255, 196, 112, 112)
#define Brown				D3DCOLOR_ARGB(255, 168, 099, 020)
#define Teal				D3DCOLOR_ARGB(255, 038, 140, 140)
#define Lime				D3DCOLOR_ARGB(255, 050, 205, 050)
#define ElectricLime		D3DCOLOR_ARGB(255, 204, 255, 000)
#define Gold				D3DCOLOR_ARGB(255, 255, 215, 000)
#define OrangeRed			D3DCOLOR_ARGB(255, 255, 69, 0)
#define GreenYellow			D3DCOLOR_ARGB(255, 173, 255, 047)
#define AquaMarine			D3DCOLOR_ARGB(255, 127, 255, 212)
#define SkyBlue				D3DCOLOR_ARGB(255, 000, 191, 255)
#define SlateBlue			D3DCOLOR_ARGB(255, 132, 112, 255)
#define Crimson				D3DCOLOR_ARGB(255, 220, 020, 060)
#define DarkOliveGreen		D3DCOLOR_ARGB(255, 188, 238, 104)
#define PaleGreen			D3DCOLOR_ARGB(255, 154, 255, 154)
#define DarkGoldenRod		D3DCOLOR_ARGB(255, 255, 185, 015)
#define FireBrick			D3DCOLOR_ARGB(255, 255, 048, 048)
#define DarkBlue			D3DCOLOR_ARGB(255, 000, 000, 204)
#define DarkerBlue			D3DCOLOR_ARGB(255, 000, 000, 153)
#define DarkYellow			D3DCOLOR_ARGB(255, 255, 204, 000)
#define LightYellow			D3DCOLOR_ARGB(255, 255, 255, 153)
#define DarkOutline			D3DCOLOR_ARGB(255, 37,   48,  52)
#define TBlack				D3DCOLOR_ARGB(180, 000, 000, 000) 
*/

#define White				D3DCOLOR_ARGB(255, 255, 255, 255)
#define Yellow				D3DCOLOR_ARGB(255, 255, 255, 0)
#define TBlack				D3DCOLOR_ARGB(180, 0, 0, 0) 
#define Black				D3DCOLOR_ARGB(255, 0, 0, 0) 
#define Red					D3DCOLOR_ARGB(255, 255, 0, 0)
#define Green				D3DCOLOR_ARGB(255, 0, 255, 0)
#define DarkOutline			D3DCOLOR_ARGB(255, 37, 48, 52)

// mensa

int mensaSelect = 0;
int Current = true;

int PosX = 30;
int PosY = 27;

int Show = false; //off by default

POINT cPos;

#define ItemColorOn Green
#define ItemColorOff Red
#define ItemCurrent White
#define GroupColor Yellow
#define KategorieFarbe Yellow
#define ItemText White

LPD3DXFONT WinFont; //font
//bool m_bCreated = false;

int CheckTab(int x, int y, int w, int h)
{
	if (Show)
	{
		GetCursorPos(&cPos);
		ScreenToClient(GetForegroundWindow(), &cPos);
		if (cPos.x > x && cPos.x < x + w && cPos.y > y && cPos.y < y + h)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 1)
			{
				//return 1; //disabled mouse selection in menu
			}
			return 2;
		}
	}
	return 0;
}

void FillRG(LPDIRECT3DDEVICE9 pDevice, int x, int y, int w, int h, D3DCOLOR color)
{
	D3DRECT rec = { x, y, x + w, y + h };
	pDevice->Clear(1, &rec, D3DCLEAR_TARGET, color, 0, 0);
}

HRESULT DrawRectangl(LPDIRECT3DDEVICE9 Device, FLOAT x, FLOAT y, FLOAT w, FLOAT h, DWORD Color)
{
	HRESULT hRet;

	const DWORD D3D_FVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	struct Vertex
	{
		float x, y, z, ht;
		DWORD vcolor;
	}
	V[4] =
	{
		{ x, (y + h), 0.0f, 0.0f, Color },
		{ x, y, 0.0f, 0.0f, Color },
		{ (x + w), (y + h), 0.0f, 0.0f, Color },
		{ (x + w), y, 0.0f, 0.0f, Color }
	};

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Device->SetPixelShader(0); //fix black color
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetFVF(D3D_FVF);
		Device->SetTexture(0, NULL);
		hRet = Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
	}

	return hRet;
}

VOID DrawBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, INT px, DWORD BorderColor)
{
	DrawRectangl(Device, x, (y + h - px), w, px, BorderColor);
	DrawRectangl(Device, x, y, px, h, BorderColor);
	DrawRectangl(Device, x, y, w, px, BorderColor);
	DrawRectangl(Device, (x + w - px), y, px, h, BorderColor);
}

VOID DrawBoxWithBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor, DWORD BorderColor)
{
	DrawRectangl(Device, x, y, w, h, BoxColor);
	DrawBorder(Device, x, y, w, h, 1, BorderColor);
}

VOID DrawBox(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor)
{
	DrawBorder(Device, x, y, w, h, 1, BoxColor);
}

void WriteTex(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	WinFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_LEFT, color);
}

void lWriteTex(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	WinFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_RIGHT, color);
}

void cWriteTex(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	WinFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_CENTER, color);
}

HRESULT DrawStrin(LPD3DXFONT WinFont, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 50, 50);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		WinFont->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = WinFont->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

void Categor(LPDIRECT3DDEVICE9 pDevice, char *text)
{
	if (Show)
	{
		int Check = CheckTab(PosX+44, (PosY+51) + (Current * 15), 190, 10);
		DWORD ColorText;

		ColorText = KategorieFarbe;

		if (Check == 2)
			ColorText = ItemCurrent;

		if (mensaSelect == Current)
			ColorText = ItemCurrent;

		WriteTex(PosX + 44, PosY+50 + (Current * 15) - 1, ColorText, text);
		lWriteTex(PosX + 236, PosY+50 + (Current * 15) - 1, ColorText, "[-]");
		Current++;
	}
}

void AddIte(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, char **opt, int MaxValue)
{
	if (Show)
	{
		int Check = CheckTab(PosX+44, (PosY+51) + (Current * 15), 190, 10);
		DWORD ColorText;

		if (var)
		{
			//DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 10, 10, Green);
			ColorText = ItemColorOn;
		}
		if (var == 0)
		{
			//DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 10, 10, Red);
			ColorText = ItemColorOff;
		}

		if (Check == 1)
		{
			var++;
			if (var > MaxValue)
				var = 0;
		}

		if (Check == 2)
			ColorText = ItemCurrent;

		if (mensaSelect == Current)
		{
			if (GetAsyncKeyState(VK_RIGHT) & 1)
			{
				var++;
				if (var > MaxValue)
					var = 0;
			}
			else if (GetAsyncKeyState(VK_LEFT) & 1)
			{
				var--;
				if (var < 0)
					var = MaxValue;
			}
		}

		if (mensaSelect == Current)
			ColorText = ItemCurrent;


		WriteTex(PosX + 44, PosY + 50 + (Current * 15) - 1, Black, text);
		WriteTex(PosX + 45, PosY + 51 + (Current * 15) - 1, ColorText, text);

		lWriteTex(PosX + 236, PosY + 50 + (Current * 15) - 1, Black, opt[var]);
		lWriteTex(PosX + 237, PosY + 51 + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}

//=====================================================================================================================

LPD3DXSPRITE pSprite0, pSprite1, pSprite2, pSprite3, pSprite4, pSprite5, pSprite6, pSprite7, pSprite8, pSprite9, pSprite10, pSprite11, pSprite12, pSprite13 = NULL; //mensa
LPD3DXSPRITE lpEsp0, lpEsp1, lpEsp2, lpEsp3, lpEsp4, lpEsp5, lpEsp6, lpEsp7, lpEsp8, lpEsp9, lpEsp10, lpEsp11, lpEsp12, lpEsp13, lpEsp14 = NULL; //esp
LPDIRECT3DTEXTURE9 pSpriteImage0, pSpriteImage1, pSpriteImage2, pSpriteImage3, pSpriteImage4, pSpriteImage5, pSpriteImage6, pSpriteImage7, pSpriteImage8, pSpriteImage9, pSpriteImage10, pSpriteImage11, pSpriteImage12, pSpriteImage13, pSpriteImage14 = NULL; //mensa
LPDIRECT3DTEXTURE9 lpEspImage0, lpEspImage1, lpEspImage2, lpEspImage3, lpEspImage4, lpEspImage5, lpEspImage6, lpEspImage7, lpEspImage8, lpEspImage9, lpEspImage10, lpEspImage11, lpEspImage12, lpEspImage13, lpEspImage14, lpEspImage15 = NULL; //esp
bool Sprite1337233802Creatd1, Sprite1337233802Creatd2 = false;

bool WinCreateSprite(IDirect3DDevice9* pd3dDevice)
{
	
	HRESULT hr;

	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\menu\\Menu0.png"), &pSpriteImage0);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame1.png"), &pSpriteImage1);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame2.png"), &pSpriteImage2);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame3.png"), &pSpriteImage3);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame4.png"), &pSpriteImage4);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame5.png"), &pSpriteImage5);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame6.png"), &pSpriteImage6);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame7.png"), &pSpriteImage7);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame8.png"), &pSpriteImage8);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame9.png"), &pSpriteImage9);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame10.png"), &pSpriteImage10);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame11.png"), &pSpriteImage11);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame12.png"), &pSpriteImage12);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\greenwater\\Frame13.png"), &pSpriteImage13);

	if (FAILED(hr))
	{
		//Log("D3DXCreateTextureFromFile failed");
		//bSpriteCreated1 = detected?
		Sprite1337233802Creatd1 = false;
		return false;
	}

	hr = D3DXCreateSprite(pd3dDevice, &pSprite0);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite1);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite2);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite3);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite4);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite5);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite6);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite7);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite8);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite9);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite10);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite11);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite12);
	hr = D3DXCreateSprite(pd3dDevice, &pSprite13);

	if (FAILED(hr))
	{
		//Log("D3DXCreateSprite1 failed");
		Sprite1337233802Creatd1 = false;
		return false;
	}

	Sprite1337233802Creatd1 = true;
	
	return true;
}

bool WinCreateSprite2(IDirect3DDevice9* pd3dDevice)
{
	
	HRESULT hr;

	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame0.png"), &lpEspImage0);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame1.png"), &lpEspImage1);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame2.png"), &lpEspImage2);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame3.png"), &lpEspImage3);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame4.png"), &lpEspImage4);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame5.png"), &lpEspImage5);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame6.png"), &lpEspImage6);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame7.png"), &lpEspImage7);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame8.png"), &lpEspImage8);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame9.png"), &lpEspImage9);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame10.png"), &lpEspImage10);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame11.png"), &lpEspImage11);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame12.png"), &lpEspImage12);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame13.png"), &lpEspImage13);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetFolderFile("stuff\\animations\\blackbluecircle\\Frame14.png"), &lpEspImage14);
	
	if (FAILED(hr))
	{
		//Log("D3DXCreateTextureFromFile failed");
		//bSpriteCreated2 = detected?
		Sprite1337233802Creatd2 = false;
		return false;
	}

	hr = D3DXCreateSprite(pd3dDevice, &lpEsp0);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp1);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp2);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp3);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp4);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp5);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp6);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp7);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp8);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp9);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp10);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp11);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp12);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp13);
	hr = D3DXCreateSprite(pd3dDevice, &lpEsp14);

	if (FAILED(hr))
	{
		//Log("D3DXCreateSprite2 failed");
		Sprite1337233802Creatd2 = false;
		return false;
	}

	Sprite1337233802Creatd2 = true;
	
	return true;
}

// COM utils
template<class COMObject>
void SafeRelease(COMObject*& pRes)
{
	IUnknown *unknown = pRes;
	if (unknown)
	{
		unknown->Release();
	}
	pRes = NULL;
}

// This will get called before Device::Clear(). If the device has been reset
// then all the work surfaces will be created again.
void PreWinClear(IDirect3DDevice9* ddevice)
{
	if (!Sprite1337233802Creatd1)
		WinCreateSprite(ddevice);

	if (!Sprite1337233802Creatd2)
		WinCreateSprite2(ddevice);
}

// Delete work surfaces when device gets reset
void DeletePicSurfaces()
{
	if (pSprite0 != NULL)
	{
		//Log("SafeRelease(pSprite)");
		SafeRelease(pSprite0);
		SafeRelease(pSprite1);
		SafeRelease(pSprite2);
		SafeRelease(pSprite3);
		SafeRelease(pSprite4);
		SafeRelease(pSprite5);
		SafeRelease(pSprite6);
		SafeRelease(pSprite7);
		SafeRelease(pSprite8);
		SafeRelease(pSprite9);
		SafeRelease(pSprite10);
		SafeRelease(pSprite11);
		SafeRelease(pSprite12);
		SafeRelease(pSprite13);
	}

	if (lpEsp0 != NULL)
	{
		//Log("SafeRelease(lpEsp)");
		SafeRelease(lpEsp0);
		SafeRelease(lpEsp1);
		SafeRelease(lpEsp2);
		SafeRelease(lpEsp3);
		SafeRelease(lpEsp4);
		SafeRelease(lpEsp5);
		SafeRelease(lpEsp6);
		SafeRelease(lpEsp7);
		SafeRelease(lpEsp8);
		SafeRelease(lpEsp9);
		SafeRelease(lpEsp10);
		SafeRelease(lpEsp11);
		SafeRelease(lpEsp12);
		SafeRelease(lpEsp13);
		SafeRelease(lpEsp14);
	}

	Sprite1337233802Creatd1 = false;
	Sprite1337233802Creatd2 = false;
}

// This gets called right before the frame is presented on-screen - Device::Present().
// First, create the display text, FPS and info message, on-screen. Then then call
// CopySurfaceToTextureBuffer() to downsample the image and copy to shared memory
void PrePresen(IDirect3DDevice9* Device, int cx, int cy)
{
	int textOffsetLeft;

	//draw sprite
	if (Sprite1337233802Creatd1)
	{
		if (pSprite0 != NULL)
		{
			D3DXVECTOR3 position;
			position.x = (float)cx;
			position.y = (float)cy;
			position.z = 0.0f;

			textOffsetLeft = (int)position.x; //for later to offset text from image

			//pSprite1->Begin(D3DXSPRITE_ALPHABLEND);
			//pSprite1->Draw(pSpriteImage1, NULL, NULL, &position, 0xFFFFFFFF);
			//pSprite1->End();

			//position.x = 20.0f; //79.0f;
			//position.y = 20.0f;
			//position.z = 0.0f;

			//mensa background scale
			D3DXMATRIX scaleMatrix0;
			D3DXMATRIX transMatrix0;
			D3DXMatrixScaling(&scaleMatrix0, 1.0f, 0.6f, 1.0f);
			D3DXMatrixTranslation(&transMatrix0, position.x, position.y, position.z);
			D3DXMatrixMultiply(&transMatrix0, &scaleMatrix0, &transMatrix0);

			//draw mensa background pic
			pSprite0->SetTransform(&transMatrix0);
			pSprite0->Begin(D3DXSPRITE_ALPHABLEND);
			pSprite0->Draw(pSpriteImage0, NULL, NULL, &position, 0xFFFFFFFF);
			pSprite0->End();

			//anim timer
			dwTime0 = GetTickCount() / 60;//50 speed

			//setting the starttime in ms
			if (dwTime0 - dwStartTime0 > 12)
				dwStartTime0 = GetTickCount() / 60;

			position.x = 49.0f; //76.0f;
			position.y = 119.0f;
			position.z = 0.0f;

			//animation scale
			D3DXMATRIX scaleMatrix;
			D3DXMATRIX transMatrix;
			D3DXMatrixScaling(&scaleMatrix, 1.51f, 0.755f, 1.0f);
			//D3DXMatrixTranslation(&transMatrix, ImagePos0.x, ImagePos0.y, ImagePos0.z);
			D3DXMatrixTranslation(&transMatrix, 0.0f, 0.0f, 0.0f);
			D3DXMatrixMultiply(&transMatrix, &scaleMatrix, &transMatrix);

			//draw animation foreground pics
			if (dwTime0 - dwStartTime0 == 0)
			{
				pSprite1->SetTransform(&transMatrix);
				pSprite1->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite1->Draw(pSpriteImage1, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite1->End();
			}

			if (dwTime0 - dwStartTime0 == 1)
			{
				pSprite2->SetTransform(&transMatrix);
				pSprite2->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite2->Draw(pSpriteImage2, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite2->End();
			}

			if (dwTime0 - dwStartTime0 == 2)
			{
				pSprite3->SetTransform(&transMatrix);
				pSprite3->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite3->Draw(pSpriteImage3, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite3->End();
			}

			if (dwTime0 - dwStartTime0 == 3)
			{
				pSprite4->SetTransform(&transMatrix);
				pSprite4->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite4->Draw(pSpriteImage4, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite4->End();
			}

			if (dwTime0 - dwStartTime0 == 4)
			{
				pSprite5->SetTransform(&transMatrix);
				pSprite5->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite5->Draw(pSpriteImage5, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite5->End();
			}

			if (dwTime0 - dwStartTime0 == 5)
			{
				pSprite6->SetTransform(&transMatrix);
				pSprite6->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite6->Draw(pSpriteImage6, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite6->End();
			}

			if (dwTime0 - dwStartTime0 == 6)
			{
				pSprite7->SetTransform(&transMatrix);
				pSprite7->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite7->Draw(pSpriteImage7, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite7->End();
			}

			if (dwTime0 - dwStartTime0 == 7)
			{
				pSprite8->SetTransform(&transMatrix);
				pSprite8->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite8->Draw(pSpriteImage8, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite8->End();
			}

			if (dwTime0 - dwStartTime0 == 8)
			{
				pSprite9->SetTransform(&transMatrix);
				pSprite9->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite9->Draw(pSpriteImage9, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite9->End();
			}

			if (dwTime0 - dwStartTime0 == 9)
			{
				pSprite10->SetTransform(&transMatrix);
				pSprite10->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite10->Draw(pSpriteImage10, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite10->End();
			}

			if (dwTime0 - dwStartTime0 == 10)
			{
				pSprite11->SetTransform(&transMatrix);
				pSprite11->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite11->Draw(pSpriteImage11, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite11->End();
			}

			if (dwTime0 - dwStartTime0 == 11)
			{
				pSprite12->SetTransform(&transMatrix);
				pSprite12->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite12->Draw(pSpriteImage12, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite12->End();
			}

			if (dwTime0 - dwStartTime0 == 12)
			{
				pSprite13->SetTransform(&transMatrix);
				pSprite13->Begin(D3DXSPRITE_ALPHABLEND);
				pSprite13->Draw(pSpriteImage13, NULL, NULL, &position, 0xFFFFFFFF);
				pSprite13->End();
			}
		}
	}

	// draw text
}

void PrePresen2(IDirect3DDevice9* Device, int cx, int cy)
{
	int textOffsetLeft;

	//draw sprite
	if (Sprite1337233802Creatd2)
	{
		if (lpEsp0 != NULL)
		{
			D3DXVECTOR3 position;
			position.x = (float)cx;
			position.y = (float)cy;
			position.z = 0.0f;

			textOffsetLeft = (int)position.x; //for later to offset text from image

			//draw esp background pic
			//lpEsp0->SetTransform(&transMatrix0);
			lpEsp0->Begin(D3DXSPRITE_ALPHABLEND);
			lpEsp0->Draw(lpEspImage0, NULL, NULL, &position, 0xFFFFFFFF);
			lpEsp0->End();


			//anim timer
			dwTime1 = GetTickCount() / 60;//50 speed

			//setting the starttime in ms
			if (dwTime1 - dwStartTime1 > 13)
				dwStartTime1 = GetTickCount() / 60;

		
			//draw animation foreground pics
			if (dwTime1 - dwStartTime1 == 0)
			{
				//lpEsp1->SetTransform(&transMatrix);
				lpEsp1->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp1->Draw(lpEspImage1, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp1->End();
			}

			if (dwTime1 - dwStartTime1 == 1)
			{
				//lpEsp2->SetTransform(&transMatrix);
				lpEsp2->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp2->Draw(lpEspImage2, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp2->End();
			}

			if (dwTime1 - dwStartTime1 == 2)
			{
				//lpEsp3->SetTransform(&transMatrix);
				lpEsp3->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp3->Draw(lpEspImage3, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp3->End();
			}

			if (dwTime1 - dwStartTime1 == 3)
			{
				//lpEsp4->SetTransform(&transMatrix);
				lpEsp4->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp4->Draw(lpEspImage4, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp4->End();
			}

			if (dwTime1 - dwStartTime1 == 4)
			{
				//lpEsp5->SetTransform(&transMatrix);
				lpEsp5->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp5->Draw(lpEspImage5, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp5->End();
			}

			if (dwTime1 - dwStartTime1 == 5)
			{
				//lpEsp6->SetTransform(&transMatrix);
				lpEsp6->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp6->Draw(lpEspImage6, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp6->End();
			}

			if (dwTime1 - dwStartTime1 == 6)
			{
				//lpEsp7->SetTransform(&transMatrix);
				lpEsp7->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp7->Draw(lpEspImage7, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp7->End();
			}

			if (dwTime1 - dwStartTime1 == 7)
			{
				//lpEsp8->SetTransform(&transMatrix);
				lpEsp8->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp8->Draw(lpEspImage8, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp8->End();
			}

			if (dwTime1 - dwStartTime1 == 8)
			{
				//lpEsp9->SetTransform(&transMatrix);
				lpEsp9->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp9->Draw(lpEspImage9, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp9->End();
			}

			if (dwTime1 - dwStartTime1 == 9)
			{
				//lpEsp10->SetTransform(&transMatrix);
				lpEsp10->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp10->Draw(lpEspImage10, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp10->End();
			}

			if (dwTime1 - dwStartTime1 == 10)
			{
				//lpEsp11->SetTransform(&transMatrix);
				lpEsp11->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp11->Draw(lpEspImage11, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp11->End();
			}

			if (dwTime1 - dwStartTime1 == 11)
			{
				//lpEsp12->SetTransform(&transMatrix);
				lpEsp12->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp12->Draw(lpEspImage12, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp12->End();
			}

			if (dwTime1 - dwStartTime1 == 12)
			{
				//lpEsp13->SetTransform(&transMatrix);
				lpEsp13->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp13->Draw(lpEspImage13, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp13->End();
			}

			if (dwTime1 - dwStartTime1 == 13)
			{
				//lpEsp14->SetTransform(&transMatrix);
				lpEsp14->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp14->Draw(lpEspImage14, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp14->End();
			}
		}
	}

	// draw text
}

//==========================================================================================================================

// mensa part
char *opt_OnOff[] = { "[OFF]", "[On]" };
char *opt_WhChams[] = { "[OFF]", "[On]", "[On + Glow]", "[On + Chams]" };
char *opt_Teams[] = { "[OFF]", "[Mode 1]", "[Mode 2]", "[Compatibility]" };
char *opt_Keys[] = { "[OFF]", "[Shift]", "[RMouse]", "[LMouse]", "[Ctrl]", "[Alt]", "[Space]", "[X]", "[C]" };
char *opt_Sensitivity[] = { "[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[10]", "[11]", "[12]", "[13]", "[14]", "[15]", "[16]", "[17]", "[18]", "[19]", "[20]" };
char *opt_Aimheight[] = { "[0]", "[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[10]" };
char *opt_Aimfov[] = { "[0]", "[10%]", "[20%]", "[30%]", "[40%]", "[50%]", "[60%]", "[70%]", "[80%]", "[90%]" };
char *opt_Aimpause[] = { "[0]", "[0.2sec]", "[0.4sec]", "[0.6sec]", "[0.8sec]", "[1.0sec]", "[1.2sec]", "[1.4sec]", "[1.6sec]", "[1.8sec]", "[2.0sec]" };
char *opt_autoshoot[] = { "[OFF]", "[OnKeyDown]" };

void Drawmensa(LPDIRECT3DDEVICE9 pDevice)
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		Show = !Show;

		//save settings
		SaveCfg();

		//Save("wallhack", "wallhack", wallhack, GetFolderFile("palaconfig.ini"));

		PlaySoundA(GetFolderFile("stuff\\sounds\\menu.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);
	}

	if (Show && WinFont)
	{
		if (GetAsyncKeyState(VK_UP) & 1)
			mensaSelect--;

		if (GetAsyncKeyState(VK_DOWN) & 1)
			mensaSelect++;

		//draw background
		FillRG(pDevice, 71, 64, 200, 201, TBlack);
		DrawBox(pDevice, 71, 63, 200, 1, DarkOutline);
		DrawBox(pDevice, 71, 86, 200, Current * 15, DarkOutline);
		//draw mensa pic
		PrePresen(pDevice, 20, 20);
		//cWriteTex(172, 71, White, "Paladins D3D");

		Current = 1;
		//Categor(pDevice, " [D3D]");
		AddIte(pDevice, " Wallhack", wallhack, opt_WhChams, 3);
		AddIte(pDevice, " Occlusion", occlusion, opt_OnOff, 1);
		AddIte(pDevice, " Esp", esp, opt_OnOff, 1);
		AddIte(pDevice, " Aimbot", aimbot, opt_Teams, 3);
		AddIte(pDevice, " Aimkey", aimkey, opt_Keys, 8);
		AddIte(pDevice, " Aimsens", aimsens, opt_Sensitivity, 19);
		AddIte(pDevice, " Aimfov", aimfov, opt_Aimfov, 9);
		AddIte(pDevice, " Aimheight", aimheight, opt_Aimheight, 10);
		AddIte(pDevice, " AimdelayAfterKill", aimpause, opt_Aimpause, 10);
		AddIte(pDevice, " Autoshoot", autoshoot, opt_autoshoot, 1);
		AddIte(pDevice, " Killsounds", killsounds, opt_OnOff, 1);

		if (mensaSelect >= Current)
			mensaSelect = 1;

		if (mensaSelect < 1)
			mensaSelect = 11;//Current;
	}
}

//=====================================================================================================================
/*
IDirect3DPixelShader9 *shadRed;
IDirect3DPixelShader9 *shadGreen;
//generate shader
HRESULT GenerateShader(IDirect3DDevice9 *pDevice, IDirect3DPixelShader9 **pShader, float r, float g, float b, float a, bool setzBuf)
{
	char szShader[256];
	ID3DXBuffer *pShaderBuf = NULL;
	D3DCAPS9 caps;
	pDevice->GetDeviceCaps(&caps);
	int PXSHVER1 = (D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion));
	int PXSHVER2 = (D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion));
	if (setzBuf)
		sprintf_s(szShader, "ps.%d.%d\ndef c0, %f, %f, %f, %f\nmov oC0,c0\nmov oDepth, c0.x", PXSHVER1, PXSHVER2, r, g, b, a);
	else
		sprintf_s(szShader, "ps.%d.%d\ndef c0, %f, %f, %f, %f\nmov oC0,c0", PXSHVER1, PXSHVER2, r, g, b, a);
	D3DXAssembleShader(szShader, sizeof(szShader), NULL, NULL, 0, &pShaderBuf, NULL);
	if (FAILED(pDevice->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), pShader)))return E_FAIL; //could crash
	return S_OK;
}
*/

