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

HMODULE dllHandle;

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
//IDirect3DVertexShader9* vShader;
//UINT vSize;

//pixelshader
IDirect3DPixelShader9* pShader;
UINT pSize;

//texture crc
IDirect3DTexture9* pCurrentTexture = NULL;
DWORD dwDataCRC;
int dWidth;
int dHeight;
//int dFormat;

D3DVIEWPORT9 Viewport; //use this viewport
float ScreenCenterX;
float ScreenCenterY;

//logger
bool logger = false;
int countnum = -1;

//features
int wallhack = 1;				//wallhack
int occlusion = 1;				//occlusion exploit

//aimbot settings
int aimbot = 2;
int aimkey = 2;
DWORD Daimkey = VK_RBUTTON;		//aimkey
int aimsens = 3;				//aim sensitivity, makes aim smoother
int aimfov = 6;					//aim field of view in % 
int aimheight = 2;				//aim height value for menu, low value = aims heigher, high values aims lower
int aimheightxy = 0;			//real value, aimheight * 4 + 27
int esp = 1;					//anim pic esp
//int useworldpos = 0;

//autoshoot settings
int autoshoot = 1;
unsigned int asdelay = 50;		//use x-999 (shoot for xx millisecs, looks more legit)
bool IsPressed = false;			//

//timer
DWORD frametime = timeGetTime();


//sprite
D3DXVECTOR3 ImagePos0;

//sprite timer
DWORD dwStartTime0 = 0; //time as the timer started
DWORD dwTime0 = 0; //windowsuptime

//==========================================================================================================================

// getdir & log
char dlldir[320];
char* GetDirectoryFile(char *filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirectoryFile("log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}

DWORD QuickChecksum(DWORD *pData, int size)
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

//get distance
float GetDistance(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

struct AimHPBarInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDistance;
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
void AddHPBarAim(LPDIRECT3DDEVICE9 Device, int iTeam)
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
		aimheightxy = (Viewport.Height / 50) + (aimheight * 3);//27 2

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
	float CrosshairDistance;
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
void AddHPBarEsp(LPDIRECT3DDEVICE9 Device, int iTeam)
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
	w = mvp[12] * world._14 + mvp[13] * world._24 + mvp[14] * world._34 + mvp[15];

	if (w > 0.01f)
	{
		aimheightxy = (Viewport.Height / 50) + (aimheight * 3);//27 2

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
	float CrosshairDistance;
};
std::vector<AimTBarInfo_t>AimTBarInfo;

void AddTBarAim(LPDIRECT3DDEVICE9 Device, int iTeam)
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
		aimheightxy = (Viewport.Height / 50) + (aimheight * 5);//27 4

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
	D3DXMATRIX projMatrix, m1;
	D3DXVECTOR4 position, input;

	float VWidth = Viewport.Width * 0.5f;
	float VHeight = Viewport.Height * 0.5f;
	Device->GetVertexShaderConstantF(6, projMatrix, 4);

	//input.y = 1; //height

	D3DXMatrixTranspose(&projMatrix, &projMatrix);
	D3DXVec4Transform(&position, &input, &projMatrix);

	position.x = input.x * projMatrix._11 + input.y * projMatrix._21 + input.z * projMatrix._31 + projMatrix._41;
	position.y = input.x * projMatrix._12 + input.y * projMatrix._22 + input.z * projMatrix._32 + projMatrix._42;
	position.z = input.x * projMatrix._13 + input.y * projMatrix._23 + input.z * projMatrix._33 + projMatrix._43;
	position.w = input.x * projMatrix._14 + input.y * projMatrix._24 + input.z * projMatrix._34 + projMatrix._44;

	//if (projMatrix._44 > -0.1f)
	//{
		//float RealDistance = position.w;

		xx = Viewport.X + VWidth - VWidth * position.x / position.w;
		yy = Viewport.Y + VHeight + VHeight * position.y / position.w; //+ not -

		AimTBarInfo_t pAimTBarInfo = { static_cast<float>(xx), static_cast<float>(yy), iTeam };
		AimTBarInfo.push_back(pAimTBarInfo);
	//}
	*/

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
// Desc: Saves Menu Item states for later Restoration
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
// Desc: Loads Menu Item States From Previously Saved File
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
void SaveSettings()
{
	ofstream fout;
	fout.open(GetDirectoryFile("palasettings.ini"), ios::trunc);
	fout << "Wallhack " << wallhack << endl;
	fout << "Occlusion " << occlusion << endl;
	fout << "Esp " << esp << endl;
	fout << "Aimbot " << aimbot << endl;
	fout << "Aimkey " << aimkey << endl;
	fout << "Aimsens " << aimsens << endl;
	fout << "Aimfov " << aimfov << endl;
	fout << "Aimheight " << aimheight << endl;
	//fout << "Useworldpos " << useworldpos << endl;
	fout << "Autoshoot " << autoshoot << endl;
	fout.close();
}

void LoadSettings()
{
	ifstream fin;
	string Word = "";
	fin.open(GetDirectoryFile("palasettings.ini"), ifstream::in);
	fin >> Word >> wallhack;
	fin >> Word >> occlusion;
	fin >> Word >> esp;
	fin >> Word >> aimbot;
	fin >> Word >> aimkey;
	fin >> Word >> aimsens;
	fin >> Word >> aimfov;
	fin >> Word >> aimheight;
	//fin >> Word >> useworldpos;
	fin >> Word >> autoshoot;
	fin.close();
}

//==========================================================================================================================

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


// menu

int MenuSelection = 0;
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

LPD3DXFONT pFont; //font
bool m_bCreated = false;

int CheckTabs(int x, int y, int w, int h)
{
	if (Show)
	{
		GetCursorPos(&cPos);
		ScreenToClient(GetForegroundWindow(), &cPos);
		if (cPos.x > x && cPos.x < x + w && cPos.y > y && cPos.y < y + h)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 1)
			{
				return 1;
			}
			return 2;
		}
	}
	return 0;
}

void FillRGB(LPDIRECT3DDEVICE9 pDevice, int x, int y, int w, int h, D3DCOLOR color)
{
	D3DRECT rec = { x, y, x + w, y + h };
	pDevice->Clear(1, &rec, D3DCLEAR_TARGET, color, 0, 0);
}

HRESULT DrawRectangle(LPDIRECT3DDEVICE9 Device, FLOAT x, FLOAT y, FLOAT w, FLOAT h, DWORD Color)
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
	DrawRectangle(Device, x, (y + h - px), w, px, BorderColor);
	DrawRectangle(Device, x, y, px, h, BorderColor);
	DrawRectangle(Device, x, y, w, px, BorderColor);
	DrawRectangle(Device, (x + w - px), y, px, h, BorderColor);
}

VOID DrawBoxWithBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor, DWORD BorderColor)
{
	DrawRectangle(Device, x, y, w, h, BoxColor);
	DrawBorder(Device, x, y, w, h, 1, BorderColor);
}

VOID DrawBox(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor)
{
	DrawBorder(Device, x, y, w, h, 1, BoxColor);
}

void WriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_LEFT, color);
}

void lWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_RIGHT, color);
}

void cWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_CENTER, color);
}

HRESULT DrawString(LPD3DXFONT pFont, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
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
		pFont->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = pFont->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

void Category(LPDIRECT3DDEVICE9 pDevice, char *text)
{
	if (Show)
	{
		int Check = CheckTabs(PosX+44, (PosY+51) + (Current * 15), 190, 10);
		DWORD ColorText;

		ColorText = KategorieFarbe;

		if (Check == 2)
			ColorText = ItemCurrent;

		if (MenuSelection == Current)
			ColorText = ItemCurrent;

		WriteText(PosX + 44, PosY+50 + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX + 236, PosY+50 + (Current * 15) - 1, ColorText, "[-]");
		Current++;
	}
}

void AddItem(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, char **opt, int MaxValue)
{
	if (Show)
	{
		int Check = CheckTabs(PosX+44, (PosY+51) + (Current * 15), 190, 10);
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

		if (MenuSelection == Current)
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

		if (MenuSelection == Current)
			ColorText = ItemCurrent;


		WriteText(PosX + 44, PosY+50 + (Current * 15) - 1, Grey, text);
		WriteText(PosX + 45, PosY + 51 + (Current * 15) - 1, ColorText, text);

		lWriteText(PosX + 236, PosY+50 + (Current * 15) - 1, Grey, opt[var]);
		lWriteText(PosX + 237, PosY + 51 + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}

//=====================================================================================================================

LPD3DXSPRITE lpSprite0, lpSprite1, lpSprite2, lpSprite3, lpSprite4, lpSprite5, lpSprite6, lpSprite7, lpSprite8, lpSprite9, lpSprite10, lpSprite11, lpSprite12, lpSprite13 = NULL; //menu
LPD3DXSPRITE lpEsp0, lpEsp1, lpEsp2, lpEsp3, lpEsp4, lpEsp5, lpEsp6, lpEsp7, lpEsp8, lpEsp9, lpEsp10, lpEsp11, lpEsp12, lpEsp13, lpEsp14 = NULL; //esp
LPDIRECT3DTEXTURE9 lpSpriteImage0, lpSpriteImage1, lpSpriteImage2, lpSpriteImage3, lpSpriteImage4, lpSpriteImage5, lpSpriteImage6, lpSpriteImage7, lpSpriteImage8, lpSpriteImage9, lpSpriteImage10, lpSpriteImage11, lpSpriteImage12, lpSpriteImage13, lpSpriteImage14 = NULL; //menu
LPDIRECT3DTEXTURE9 lpEspImage0, lpEspImage1, lpEspImage2, lpEspImage3, lpEspImage4, lpEspImage5, lpEspImage6, lpEspImage7, lpEspImage8, lpEspImage9, lpEspImage10, lpEspImage11, lpEspImage12, lpEspImage13, lpEspImage14, lpEspImage15 = NULL; //esp
bool bSpriteCreated1, bSpriteCreated2 = false;

bool CreateOverlaySprite(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\menu\\Menu0.png"), &lpSpriteImage0);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame1.png"), &lpSpriteImage1);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame2.png"), &lpSpriteImage2);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame3.png"), &lpSpriteImage3);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame4.png"), &lpSpriteImage4);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame5.png"), &lpSpriteImage5);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame6.png"), &lpSpriteImage6);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame7.png"), &lpSpriteImage7);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame8.png"), &lpSpriteImage8);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame9.png"), &lpSpriteImage9);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame10.png"), &lpSpriteImage10);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame11.png"), &lpSpriteImage11);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame12.png"), &lpSpriteImage12);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\greenwater\\Frame13.png"), &lpSpriteImage13);

	if (FAILED(hr))
	{
		//Log("D3DXCreateTextureFromFile failed");
		bSpriteCreated1 = false;
		return false;
	}

	hr = D3DXCreateSprite(pd3dDevice, &lpSprite0);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite1);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite2);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite3);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite4);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite5);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite6);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite7);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite8);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite9);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite10);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite11);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite12);
	hr = D3DXCreateSprite(pd3dDevice, &lpSprite13);

	if (FAILED(hr))
	{
		//Log("D3DXCreateSprite1 failed");
		bSpriteCreated1 = false;
		return false;
	}

	bSpriteCreated1 = true;

	return true;
}

bool CreateOverlaySprite2(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame0.png"), &lpEspImage0);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame1.png"), &lpEspImage1);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame2.png"), &lpEspImage2);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame3.png"), &lpEspImage3);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame4.png"), &lpEspImage4);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame5.png"), &lpEspImage5);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame6.png"), &lpEspImage6);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame7.png"), &lpEspImage7);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame8.png"), &lpEspImage8);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame9.png"), &lpEspImage9);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame10.png"), &lpEspImage10);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame11.png"), &lpEspImage11);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame12.png"), &lpEspImage12);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame13.png"), &lpEspImage13);
	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("stuff\\animations\\blackbluecircle\\Frame14.png"), &lpEspImage14);

	if (FAILED(hr))
	{
		//Log("D3DXCreateTextureFromFile failed");
		bSpriteCreated2 = false;
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
		bSpriteCreated2 = false;
		return false;
	}

	bSpriteCreated2 = true;

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
void PreClear(IDirect3DDevice9* device)
{
	if (!bSpriteCreated1)
		CreateOverlaySprite(device);

	if (!bSpriteCreated2)
		CreateOverlaySprite2(device);
}

// Delete work surfaces when device gets reset
void DeleteRenderSurfaces()
{
	if (lpSprite0 != NULL)
	{
		//Log("SafeRelease(lpSprite)");
		SafeRelease(lpSprite0);
		SafeRelease(lpSprite1);
		SafeRelease(lpSprite2);
		SafeRelease(lpSprite3);
		SafeRelease(lpSprite4);
		SafeRelease(lpSprite5);
		SafeRelease(lpSprite6);
		SafeRelease(lpSprite7);
		SafeRelease(lpSprite8);
		SafeRelease(lpSprite9);
		SafeRelease(lpSprite10);
		SafeRelease(lpSprite11);
		SafeRelease(lpSprite12);
		SafeRelease(lpSprite13);
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

	bSpriteCreated1 = false;
	bSpriteCreated2 = false;
}

// This gets called right before the frame is presented on-screen - Device::Present().
// First, create the display text, FPS and info message, on-screen. Then then call
// CopySurfaceToTextureBuffer() to downsample the image and copy to shared memory
void PrePresent(IDirect3DDevice9* Device, int cx, int cy)
{
	int textOffsetLeft;

	//draw sprite
	if (bSpriteCreated1)
	{
		if (lpSprite0 != NULL)
		{
			D3DXVECTOR3 position;
			position.x = (float)cx;
			position.y = (float)cy;
			position.z = 0.0f;

			textOffsetLeft = (int)position.x; //for later to offset text from image

			//lpSprite1->Begin(D3DXSPRITE_ALPHABLEND);
			//lpSprite1->Draw(lpSpriteImage1, NULL, NULL, &position, 0xFFFFFFFF);
			//lpSprite1->End();

			position.x = 20.0f; //79.0f;
			position.y = 20.0f;
			position.z = 0.0f;

			//menu background scale
			D3DXMATRIX scaleMatrix0;
			D3DXMATRIX transMatrix0;
			D3DXMatrixScaling(&scaleMatrix0, 1.0f, 0.6f, 1.0f);
			D3DXMatrixTranslation(&transMatrix0, position.x, position.y, position.z);
			D3DXMatrixMultiply(&transMatrix0, &scaleMatrix0, &transMatrix0);

			//draw menu background pic
			lpSprite0->SetTransform(&transMatrix0);
			lpSprite0->Begin(D3DXSPRITE_ALPHABLEND);
			lpSprite0->Draw(lpSpriteImage0, NULL, NULL, &position, 0xFFFFFFFF);
			lpSprite0->End();

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
			D3DXMatrixTranslation(&transMatrix, ImagePos0.x, ImagePos0.y, ImagePos0.z);
			D3DXMatrixMultiply(&transMatrix, &scaleMatrix, &transMatrix);

			//draw animation foreground pics
			if (dwTime0 - dwStartTime0 == 0)
			{
				lpSprite1->SetTransform(&transMatrix);
				lpSprite1->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite1->Draw(lpSpriteImage1, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite1->End();
			}

			if (dwTime0 - dwStartTime0 == 1)
			{
				lpSprite2->SetTransform(&transMatrix);
				lpSprite2->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite2->Draw(lpSpriteImage2, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite2->End();
			}

			if (dwTime0 - dwStartTime0 == 2)
			{
				lpSprite3->SetTransform(&transMatrix);
				lpSprite3->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite3->Draw(lpSpriteImage3, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite3->End();
			}

			if (dwTime0 - dwStartTime0 == 3)
			{
				lpSprite4->SetTransform(&transMatrix);
				lpSprite4->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite4->Draw(lpSpriteImage4, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite4->End();
			}

			if (dwTime0 - dwStartTime0 == 4)
			{
				lpSprite5->SetTransform(&transMatrix);
				lpSprite5->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite5->Draw(lpSpriteImage5, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite5->End();
			}

			if (dwTime0 - dwStartTime0 == 5)
			{
				lpSprite6->SetTransform(&transMatrix);
				lpSprite6->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite6->Draw(lpSpriteImage6, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite6->End();
			}

			if (dwTime0 - dwStartTime0 == 6)
			{
				lpSprite7->SetTransform(&transMatrix);
				lpSprite7->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite7->Draw(lpSpriteImage7, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite7->End();
			}

			if (dwTime0 - dwStartTime0 == 7)
			{
				lpSprite8->SetTransform(&transMatrix);
				lpSprite8->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite8->Draw(lpSpriteImage8, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite8->End();
			}

			if (dwTime0 - dwStartTime0 == 8)
			{
				lpSprite9->SetTransform(&transMatrix);
				lpSprite9->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite9->Draw(lpSpriteImage9, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite9->End();
			}

			if (dwTime0 - dwStartTime0 == 9)
			{
				lpSprite10->SetTransform(&transMatrix);
				lpSprite10->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite10->Draw(lpSpriteImage10, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite10->End();
			}

			if (dwTime0 - dwStartTime0 == 10)
			{
				lpSprite11->SetTransform(&transMatrix);
				lpSprite11->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite11->Draw(lpSpriteImage11, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite11->End();
			}

			if (dwTime0 - dwStartTime0 == 11)
			{
				lpSprite12->SetTransform(&transMatrix);
				lpSprite12->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite12->Draw(lpSpriteImage12, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite12->End();
			}

			if (dwTime0 - dwStartTime0 == 12)
			{
				lpSprite13->SetTransform(&transMatrix);
				lpSprite13->Begin(D3DXSPRITE_ALPHABLEND);
				lpSprite13->Draw(lpSpriteImage13, NULL, NULL, &position, 0xFFFFFFFF);
				lpSprite13->End();
			}
		}
	}

	// draw text
}

void PrePresent2(IDirect3DDevice9* Device, int cx, int cy)
{
	int textOffsetLeft;

	//draw sprite
	if (bSpriteCreated2)
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
			dwTime0 = GetTickCount() / 60;//50 speed

			//setting the starttime in ms
			if (dwTime0 - dwStartTime0 > 13)
				dwStartTime0 = GetTickCount() / 60;


			//draw animation foreground pics
			if (dwTime0 - dwStartTime0 == 0)
			{
				//lpEsp1->SetTransform(&transMatrix);
				lpEsp1->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp1->Draw(lpEspImage1, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp1->End();
			}

			if (dwTime0 - dwStartTime0 == 1)
			{
				//lpEsp2->SetTransform(&transMatrix);
				lpEsp2->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp2->Draw(lpEspImage2, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp2->End();
			}

			if (dwTime0 - dwStartTime0 == 2)
			{
				//lpEsp3->SetTransform(&transMatrix);
				lpEsp3->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp3->Draw(lpEspImage3, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp3->End();
			}

			if (dwTime0 - dwStartTime0 == 3)
			{
				//lpEsp4->SetTransform(&transMatrix);
				lpEsp4->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp4->Draw(lpEspImage4, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp4->End();
			}

			if (dwTime0 - dwStartTime0 == 4)
			{
				//lpEsp5->SetTransform(&transMatrix);
				lpEsp5->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp5->Draw(lpEspImage5, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp5->End();
			}

			if (dwTime0 - dwStartTime0 == 5)
			{
				//lpEsp6->SetTransform(&transMatrix);
				lpEsp6->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp6->Draw(lpEspImage6, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp6->End();
			}

			if (dwTime0 - dwStartTime0 == 6)
			{
				//lpEsp7->SetTransform(&transMatrix);
				lpEsp7->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp7->Draw(lpEspImage7, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp7->End();
			}

			if (dwTime0 - dwStartTime0 == 7)
			{
				//lpEsp8->SetTransform(&transMatrix);
				lpEsp8->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp8->Draw(lpEspImage8, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp8->End();
			}

			if (dwTime0 - dwStartTime0 == 8)
			{
				//lpEsp9->SetTransform(&transMatrix);
				lpEsp9->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp9->Draw(lpEspImage9, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp9->End();
			}

			if (dwTime0 - dwStartTime0 == 9)
			{
				//lpEsp10->SetTransform(&transMatrix);
				lpEsp10->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp10->Draw(lpEspImage10, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp10->End();
			}

			if (dwTime0 - dwStartTime0 == 10)
			{
				//lpEsp11->SetTransform(&transMatrix);
				lpEsp11->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp11->Draw(lpEspImage11, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp11->End();
			}

			if (dwTime0 - dwStartTime0 == 11)
			{
				//lpEsp12->SetTransform(&transMatrix);
				lpEsp12->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp12->Draw(lpEspImage12, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp12->End();
			}

			if (dwTime0 - dwStartTime0 == 12)
			{
				//lpEsp13->SetTransform(&transMatrix);
				lpEsp13->Begin(D3DXSPRITE_ALPHABLEND);
				lpEsp13->Draw(lpEspImage13, NULL, NULL, &position, 0xFFFFFFFF);
				lpEsp13->End();
			}

			if (dwTime0 - dwStartTime0 == 13)
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

// menu part
char *opt_OnOff[] = { "[OFF]", "[On]" };
char *opt_WhChams[] = { "[OFF]", "[On]", "[On + Glow]", "[On + Chams]" };
char *opt_Teams[] = { "[OFF]", "[Mode 1]", "[Mode 2]", "[Compatibility]" };
char *opt_Keys[] = { "[OFF]", "[Shift]", "[RMouse]", "[LMouse]", "[Ctrl]", "[Alt]", "[Space]", "[X]", "[C]" };
char *opt_Sensitivity[] = { "[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[10]", "[11]", "[12]", "[13]", "[14]", "[15]", "[16]", "[17]", "[18]", "[19]", "[20]" };
char *opt_Aimheight[] = { "[0]", "[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[10]" };
char *opt_Aimfov[] = { "[0]", "[10%]", "[20%]", "[30%]", "[40%]", "[50%]", "[60%]", "[70%]", "[80%]", "[90%]" };
char *opt_Autoshoot[] = { "[OFF]", "[OnKeyDown]" };

void BuildMenu(LPDIRECT3DDEVICE9 pDevice)
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		Show = !Show;

		//save settings
		SaveSettings();

		//Save("Wallhack", "Wallhack", wallhack, GetDirectoryFile("palasettings.ini"));

		PlaySoundA(GetDirectoryFile("stuff\\sounds\\menu.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);
	}

	if (Show && pFont)
	{
		//Shift|RMouse|LMouse|Ctrl|Alt|Space|X|C
		if (aimkey == 0) Daimkey = 0;
		if (aimkey == 1) Daimkey = VK_SHIFT;
		if (aimkey == 2) Daimkey = VK_RBUTTON;
		if (aimkey == 3) Daimkey = VK_LBUTTON;
		if (aimkey == 4) Daimkey = VK_CONTROL;
		if (aimkey == 5) Daimkey = VK_MENU;
		if (aimkey == 6) Daimkey = VK_SPACE;
		if (aimkey == 7) Daimkey = 0x58; //X
		if (aimkey == 8) Daimkey = 0x43; //C

		if (GetAsyncKeyState(VK_UP) & 1)
			MenuSelection--;

		if (GetAsyncKeyState(VK_DOWN) & 1)
			MenuSelection++;

		//draw background
		FillRGB(pDevice, 71, 64, 200, 172, TBlack);

		//draw menu pic
		PrePresent(pDevice, 20, 20);

		//DrawBox(pDevice, 71, 63, 200, 1, DarkOutline);
		cWriteText(170, 71, White, "Paladins D3D");
		//DrawBox(pDevice, 71, 86, 200, Current * 15, DarkOutline);

		Current = 1;
		//Category(pDevice, " [D3D]");
		AddItem(pDevice, " Wallhack", wallhack, opt_WhChams, 3);
		AddItem(pDevice, " Occlusion", occlusion, opt_OnOff, 1);
		AddItem(pDevice, " Esp", esp, opt_OnOff, 1);
		AddItem(pDevice, " Aimbot", aimbot, opt_Teams, 3);
		AddItem(pDevice, " Aimkey", aimkey, opt_Keys, 8);
		AddItem(pDevice, " Aimsens", aimsens, opt_Sensitivity, 19);
		AddItem(pDevice, " Aimfov", aimfov, opt_Aimfov, 9);
		AddItem(pDevice, " Aimheight", aimheight, opt_Aimheight, 10);
		//AddItem(pDevice, " UseWorldPos", useworldpos, opt_OnOff, 1);
		AddItem(pDevice, " Autoshoot", autoshoot, opt_Autoshoot, 1);

		//if (MenuSelection >= Current)
			//MenuSelection = 1;

		if (MenuSelection > 9)
			MenuSelection = 1;//Current;

		if (MenuSelection < 1)
			MenuSelection = 9;//Current;
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
	if (FAILED(pDevice->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), pShader)))return E_FAIL;
	return S_OK;
}
*/

