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

//DX Includes
#include <DirectXMath.h>
using namespace DirectX;

//#include <d3dx9.h>
//#pragma comment(lib, "d3dx9.lib") 
#pragma comment(lib, "winmm.lib")
#include "MinHook/include/MinHook.h" //detour
using namespace std;

#pragma warning (disable: 4244) //

//==========================================================================================================================

HMODULE Hand;

//Stride_
UINT Stride_;

//elementcount
D3DVERTEXELEMENT9 decl_[MAXD3DDECLLENGTH];
UINT num__elements;

//vertexshaderconstantf
UINT mStart__register;
UINT mVector__Count;

//vdesc.Size
D3DVERTEXBUFFER_DESC vdesc;

//vertexshader
IDirect3DVertexShader9* vShader_;
UINT vSize_;

//pixelshader
IDirect3DPixelShader9* pShader_;
UINT pSize_;

//texture crc
IDirect3DTexture9* pCurrentTex = NULL;
DWORD CRC;
int dWidth_;
int dHeight_;
//int dFormat_;
int mStage_;

//features
int wall__hack = 1;				//wall__hack
int occlu__sion = 1;			//occlu__sion exploit

//aim__bot settings
int aim__bot = 1;
int aim__key = 2;
DWORD Daim__key = VK_RBUTTON;	//aim__key
int aim__sens = 3;				//aim sensitivity, makes aim smoother
int aim__fov = 6;				//aim field of view in % 
int aim__height = 2;			//aim height value for mensa, low value = aims heigher, high values aims lower
int aim__heightxy = 1;			//real value, aim__height * 4 + 27
int es__p = 10;					//es__p

int useaim__human = 0;			//human aim sensitivity value
int aim__pause = 5;
bool after__kill = false;
bool smooth__on = false;

bool maxcharge = false;
int snipermode = 0;
int triggerdist = 20;
unsigned int scopeoff = 50;

bool sound_ = false;
int kill__sounds = 0;

//auto__shoot settings
int auto__shoot = 1;
unsigned int asdelay_ = 49;		//use x-999 (shoot for xx millisecs, looks more legit)
bool Is__Pressed = false;		//

//timer
DWORD astime_ = timeGetTime();	//auto_shoot
DWORD farmeafter__kill = timeGetTime(); //aim__pause
DWORD frame__smooth = timeGetTime(); //aimpause
DWORD sound__pause = timeGetTime(); //kill__sounds
DWORD frametime = timeGetTime(); //kinessa aim/autoshoot

//init only once
bool FirstInit = true;
bool DoInit = true;

D3DVIEWPORT9 View__port; //use this Viewport
float ScreenCX;
float ScreenCY;

#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }

//logger
bool logger = false;
int countnum = -1;

//==========================================================================================================================

// getdir & log
char dir[320];
char* GetDirFile(char *name)
{
	static char pldir[320];
	strcpy_s(pldir, dir);
	strcat_s(pldir, name);
	return pldir;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirFile("log.txt"), ios::app);
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

struct AimHPBarIno_t_
{
	float vOutX_, vOutY_;
	INT       w2sStartRegister;
	float Crosshair__Distance;
};
std::vector<AimHPBarIno_t_>AimHPBar__Info;
//float RealDistance;

// Parameters:
//
//   float4 mvp_[4];
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   mvp_          c8       4
//w2s for health bars
void HPBarAim_(LPDIRECT3DDEVICE9 Device, int w2sStartRegister) //aimbot 1
{
	//======================
	//w2s

	//vec3 vStart;
	//vec3 vOut;
	//float *flMatrix;
	//DirectX::XMMATRIX Matrix = DirectX::XMMatrixMultiply((FXMMATRIX)*matWorldView, (FXMMATRIX)*matProj);

	DirectX::XMVECTOR Pos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);//verdun(game) aimheight with pre aim 1.0
	XMMATRIX mvp_;
	//XMFLOAT4X4 world_;
	XMMATRIX world_;

	Device->GetVertexShaderConstantF(w2sStartRegister, (float*)&mvp_, 4);//8mvp_
	Device->GetVertexShaderConstantF(240, (float*)&world_, 4);//world_ 240-243, 247-252

	float *mvp = (float*)&mvp_;
	//XMMATRIX World((float*)&world_);
	
	//transpose or not, depends on the game
	//DirectX::XMMATRIX Matrixx = DirectX::XMMatrixTranspose(Matrix); //transpose

	//flMatrix = (float*)&Matrix;

	//access individual elements
	//XMFLOAT4X4 tmp;
	//XMStoreFloat4x4(&tmp, i);
	//float j = tmp.m[0][0]; // or float j = tmp._11
	
	Pos.m128_f32[0] = mvp[0] * world_.r[0].m128_f32[4] + mvp[1] * world_.r[1].m128_f32[4] + mvp[2] * world_.r[2].m128_f32[4] + mvp[3];
	Pos.m128_f32[1] = mvp[4] * world_.r[0].m128_f32[4] + mvp[5] * world_.r[1].m128_f32[4] + mvp[6] * world_.r[2].m128_f32[4] + mvp[7];

	Pos.m128_f32[3] = mvp[12] * world_.r[0].m128_f32[4] + mvp[13] * world_.r[1].m128_f32[4] + mvp[14] * world_.r[2].m128_f32[4] + mvp[15];//w

	if (Pos.m128_f32[3] > 0.01f)
	{
		if (w2sStartRegister == 8)
			aim__heightxy = ((float)View__port.Height * 0.01f) + (aim__height * 3);//0.03f
		if (w2sStartRegister == 6)
			aim__heightxy = ((float)View__port.Height * 0.01f) + (aim__height * 5);

		float x = (float)View__port.Width / 2.0f;
		float y = (float)View__port.Height / 2.0f;

		x += 0.5f * Pos.m128_f32[0] * (float)View__port.Width + 0.5f;
		y -= 0.5f * Pos.m128_f32[1] * (float)View__port.Height - (float)aim__heightxy; //+ 0.5f;

		Pos.m128_f32[0] = x;
		Pos.m128_f32[1] = y;
	}
	else
	{
		Pos.m128_f32[0] = -1.0f;
		Pos.m128_f32[1] = -1.0f;
	}

	AimHPBarIno_t_ pAimHPBar__Info = { static_cast<float>(Pos.m128_f32[0]), static_cast<float>(Pos.m128_f32[1]), w2sStartRegister };
	AimHPBar__Info.push_back(pAimHPBar__Info);
	//======================
}

//==========================================================================================================================

//-----------------------------------------------------------------------------
// Name: Save()
// Desc: Saves mensu Item states for later Restoration
//-----------------------------------------------------------------------------
/*
void Save(char* szSection, char* szKey, int iValue, LPCSTR file)
{
char szValue[255];
sprintf_s(szValue, "%d", iValue);
WriteateProfileString(szSection, szKey, szValue, file);
}
*/
//-----------------------------------------------------------------------------
// Name: Load()
// Desc: Loads mensu Item States from_ Previously Saved File
//-----------------------------------------------------------------------------
/*
int Load(char* szSection, char* szKey, int iDefaultValue, LPCSTR file)
{
int iResult = GetateProfileInt(szSection, szKey, iDefaultValue, file);
return iResult;
}
*/

#include <string>
#include <fstream>
void SaveCfg()
{
	ofstream fout;
	fout.open(GetDirFile("palad3d.ini"), ios::trunc);
	fout << "wall__hack " << wall__hack << endl;
	fout << "occlu__sion " << occlu__sion << endl;
	fout << "es__p " << es__p << endl;
	fout << "aim__bot " << aim__bot << endl;
	fout << "aim__key " << aim__key << endl;
	fout << "aim__sens " << aim__sens << endl;
	fout << "aim__fov " << aim__fov << endl;
	fout << "aim__height " << aim__height << endl;
	fout << "AimHuman " << useaim__human << endl;
	fout << "auto__shoot " << auto__shoot << endl;
	fout << "kill__sounds " << kill__sounds << endl;
	fout << "Snipermode " << snipermode << endl;
	fout.close();
}

void LoadCfg()
{
	ifstream fin;
	string Word = "";
	fin.open(GetDirFile("palad3d.ini"), ifstream::in);
	fin >> Word >> wall__hack;
	fin >> Word >> occlu__sion;
	fin >> Word >> es__p;
	fin >> Word >> aim__bot;
	fin >> Word >> aim__key;
	fin >> Word >> aim__sens;
	fin >> Word >> aim__fov;
	fin >> Word >> aim__height;
	fin >> Word >> useaim__human;
	fin >> Word >> auto__shoot;
	fin >> Word >> kill__sounds;
	fin >> Word >> snipermode;
	fin.close();
}

//==========================================================================================================================
//#define White				D3DCOLOR_ARGB(255, 255, 255, 255)
//#define Yellow				D3DCOLOR_ARGB(255, 255, 255, 0)
//#define TBlack				D3DCOLOR_ARGB(180, 0, 0, 0) 
//#define Black				D3DCOLOR_ARGB(255, 0, 0, 0) 
//#define Red					D3DCOLOR_ARGB(255, 255, 0, 0)
//#define Green				D3DCOLOR_ARGB(255, 0, 255, 0)
//#define DarkOutline			D3DCOLOR_ARGB(255, 37, 48, 52)

// mensa

int men__uselect = 0;
int Current = true;

int PosX_ = 30;
int PosY_ = 27;

int Show = false; //off by default

POINT Pos;

//#define ItemColorOn Green
//#define ItemColorOff Red
//#define ItemCurrent White
//#define GroupColor Yellow
//#define KategorieFarbe Yellow
//#define ItemText White

LPD3DXFONT Font; //font

int CheckTab(int x, int y, int w, int h)
{
	if (Show)
	{
		GetCursorPos(&Pos);
		ScreenToClient(GetForegroundWindow(), &Pos);
		if (Pos.x > x && Pos.x < x + w && Pos.y > y && Pos.y < y + h)
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

void FillRGB(LPDIRECT3DDEVICE9 pDevice, int x, int y, int w, int h, D3DCOLOR color)
{
	D3DRECT rec = { x, y, x + w, y + h };
	pDevice->Clear(1, &rec, D3DCLEAR_TARGET, color, 0, 0);
}

HRESULT DrawRectangle(LPDIRECT3DDEVICE9 Device, int x, int y, int w, int h, DWORD Color)
{
	HRESULT hRet;

	//const DWORD D3D_FVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	struct Vertex
	{
		float x, y, z, ht;
		DWORD vcolor;
	}
	V[4] =
	{
		//{ x, (y + h), 0, 0, Color },
		//{ x, y, 0, 0, Color },
		//{ (x + w), (y + h), 0, 0, Color },
		//{ (x + w), y, 0, 0, Color }

		{ (float)x , (float)(y + h), 0.0f, 1.0f, Color },
		{ (float)x , (float)y , 0.0f, 1.0f, Color },
		{ (float)(x + w), (float)(y + h), 0.0f, 1.0f, Color },
		{ (float)(x + w), (float)y , 0.0f, 1.0f, Color }
	};

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		//Device->SetPixelShader(0); //fix black color
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
		Device->SetTexture(0, NULL);
		hRet = Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
	}

	return hRet;
}

struct CUSTOM_VERTEX
{
	float x, y, z, MaxWidth, Height;
	DWORD ColourHi, ColourLo;
};

void Slidebar(IDirect3DDevice9 *pDevice, float x, float y, float w, float h, D3DCOLOR Color)
{
	struct Vertex
	{
		float x, y, z, ht;
		DWORD Color;
	}
	V[4] = { { x, y + h, 0.0f, 0.0f, Color },{ x, y, 0.0f, 0.01f, Color },
	{ x + w, y + h, 0.0f, 0.0f, Color },{ x + w, y, 0.0f, 0.0f, Color } };
	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(0);
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
	//return;
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
	Font->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_LEFT, color);
}

void lWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	Font->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_RIGHT, color);
}

void cWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	Font->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_CENTER, color);
}

HRESULT DrawString(LPD3DXFONT Font, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
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
		Font->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = Font->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

void Category(LPDIRECT3DDEVICE9 pDevice, char *text)
{
	if (Show)
	{
		int Check = CheckTab(PosX_+44, (PosY_+51) + (Current * 15), 190, 10);
		DWORD ColorText;

		ColorText = D3DCOLOR_ARGB(255, 255, 0, 255);

		if (Check == 2)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		if (men__uselect == Current)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		WriteText(PosX_ + 44, PosY_+50 + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX_ + 236, PosY_+50 + (Current * 15) - 1, ColorText, "[-]");
		Current++;
	}
}

void AddItem(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, char **opt, int MaxValue)
{
	if (Show)
	{
		int Check = CheckTab(PosX_+44, (PosY_+51) + (Current * 15), 190, 10);
		DWORD ColorText;

		if (var)
		{
			//DrawBox(pDevice, PosX_+44, PosY_+51 + (Current * 15), 10, 10, Green);
			ColorText = D3DCOLOR_ARGB(255, 0, 255, 0);
		}
		if (var == 0)
		{
			//DrawBox(pDevice, PosX_+44, PosY_+51 + (Current * 15), 10, 10, Red);
			ColorText = D3DCOLOR_ARGB(255, 255, 0, 0);
		}

		if (Check == 1)
		{
			var++;
			if (var > MaxValue)
				var = 0;
		}

		if (Check == 2)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		if (men__uselect == Current)
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

		if (men__uselect == Current)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);


		WriteText(PosX_ + 44, PosY_ + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 50, 50, 50), text);
		WriteText(PosX_ + 45, PosY_ + 51 + (Current * 15) - 1, ColorText, text);

		lWriteText(PosX_ + 236, PosY_ + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 100, 100, 100), opt[var]);
		lWriteText(PosX_ + 237, PosY_ + 51 + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}

//==========================================================================================================================

// menu part
char *_opt_OnOff[] = { "[OFF]", "[On]" };
char *_opt_Ones__p[] = { "[OFF]", "[Line Down]", "[Line Up]" };
char *_opt_WhChams[] = { "[OFF]", "[On]", "[On + Glow]", "[On + Chams]" };
char *_opt_Teams[] = { "[OFF]", "[Mode 1]", "[Mode 2]", "[Compatibility]" };
char *_opt_Keys[] = { "[OFF]", "[Shift]", "[RMouse]", "[LMouse]", "[Ctrl]", "[Alt]", "[Space]", "[X]", "[C]" };
char *_opt_Sensitivity[] = { "[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[10]", "[11]", "[12]", "[13]", "[14]", "[15]", "[16]", "[17]", "[18]", "[19]", "[20]" };
char *_opt_aim__height[] = { "[0]", "[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[10]" };
char *_opt_aim__fov[] = { "[0]", "[10%]", "[20%]", "[30%]", "[40%]", "[50%]", "[60%]", "[70%]", "[80%]", "[90%]" };
char *_opt_Aimhuman[] = { "[OFF]", "[On sens 1]", "[On sens 2]", "[On sens 3]", "[On sens 4]", "[On sens 5]", "[On sens 6]", "[On sens 7]", "[On sens 8]", "[On sens 9]", "[On sens 10]" };
char *_opt_auto__shoot[] = { "[OFF]", "[OnKeyDown]" };

void DrawMenu(LPDIRECT3DDEVICE9 pDevice)
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		DoInit = true; //get viewport again

		Show = !Show;

		//save settings
		SaveCfg();

		//Save("wall__hack", "wall__hack", wall__hack, GetDirFile("aconfig.ini"));

		//PlaySoundA(GetDirFile("snd\\.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);
	}

	if (Show && Font)
	{
		if (GetAsyncKeyState(VK_UP) & 1)
			men__uselect--;

		if (GetAsyncKeyState(VK_DOWN) & 1)
			men__uselect++;

		//draw background
		Slidebar(pDevice, 71.0f, 86.0f, 200.0f, 194.0f, D3DCOLOR_ARGB(120, 30, 200, 200));//194 = up/down, 200 = left/right
		//FillRGB(pDevice, 71, 86, 200, 164, D3DCOLOR_ARGB(255, 0, 0, 0));
		//DrawRectangle(pDevice, 71, 86, 200, 164, D3DCOLOR_ARGB(255, 0, 0, 0));
		DrawBox(pDevice, 71, 86, 200, Current * 15, D3DCOLOR_ARGB(255, 255, 255, 255));
		//draw mensa pic
	//	PrePresent(pDevice, 20, 20);
		//cWriteText(172, 71, White, "Paladins D3D");

		Current = 1;
		//Category(pDevice, " [D3D]");
		AddItem(pDevice, " Wallhack", wall__hack, _opt_WhChams, 3);
		AddItem(pDevice, " Occlusion", occlu__sion, _opt_OnOff, 1);
		AddItem(pDevice, " Esp", es__p, _opt_aim__height, 10);
		AddItem(pDevice, " Aimbot", aim__bot, _opt_Teams, 2);
		AddItem(pDevice, " Aimkey", aim__key, _opt_Keys, 8);
		AddItem(pDevice, " A1msens", aim__sens, _opt_Sensitivity, 19);
		AddItem(pDevice, " Aimfov", aim__fov, _opt_aim__fov, 9);
		AddItem(pDevice, " Aimheight", aim__height, _opt_aim__height, 10);
		AddItem(pDevice, " Aimhuman", useaim__human, _opt_Aimhuman, 10);
		AddItem(pDevice, " Autoshoot", auto__shoot, _opt_auto__shoot, 1);
		AddItem(pDevice, " Snipermode", snipermode, _opt_OnOff, 1);
		AddItem(pDevice, " KillSounds", kill__sounds, _opt_OnOff, 1);

		if (men__uselect >= Current)
			men__uselect = 1;

		if (men__uselect < 1)
			men__uselect = 12;//Current;
	}
}

//=====================================================================================================================

//for line esp
class D3DTLVERTEX
{
public:
	FLOAT X, Y, X2, Y2;
	DWORD Color;
};

void DrawLine(IDirect3DDevice9* m_pD3Ddev, float X, float Y, float X2, float Y2, float Width, D3DCOLOR Color)
{
	D3DTLVERTEX qV[2] = {
		{ (float)X , (float)Y, 0.0f, 1.0f, Color },
		{ (float)X2 , (float)Y2 , 0.0f, 1.0f, Color },
	};
	const DWORD D3DFVF_TL = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	m_pD3Ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pD3Ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pD3Ddev->SetFVF(D3DFVF_TL);
	m_pD3Ddev->SetTexture(0, NULL);
	m_pD3Ddev->DrawPrimitiveUP(D3DPT_LINELIST, 2, qV, sizeof(D3DTLVERTEX));
}

