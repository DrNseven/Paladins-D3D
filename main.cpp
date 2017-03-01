/*
* Paladins D3D Hack Source V1.6b by Nseven

How to compile:
- download and install "Microsoft Visual Studio Express 2015 for Windows DESKTOP" https://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx

- open PhysXUpdateLoader.vcxproj (not PhysXUpdateLoader.vcxproj.filters) with Visual Studio 2015 (Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\WDExpress.exe)
- select x86(32bit) 
- compile dll, press f7 or click the green triangle

x86 compiled dll will be in paladinsd3d\Release folder

If you share your dll with others, remove dependecy on vs runtime before compiling:
- click: project -> properties -> configuration properties -> C/C++ -> code generation -> runtime library: Multi-threaded (/MT)

menu key:
- insert

Logging:
ALT + CTRL + L toggles logger
- press O to decrease values
- press P to increase, hold down P key until a texture changes
- press I to log values of changed textures
*/

#include "main.h" //less important stuff & helper funcs here

typedef HRESULT(APIENTRY *SetStreamSource)(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
HRESULT APIENTRY SetStreamSource_hook(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
SetStreamSource SetStreamSource_orig = 0;

typedef HRESULT(APIENTRY *SetVertexDeclaration)(IDirect3DDevice9*, IDirect3DVertexDeclaration9*);
HRESULT APIENTRY SetVertexDeclaration_hook(IDirect3DDevice9*, IDirect3DVertexDeclaration9*);
SetVertexDeclaration SetVertexDeclaration_orig = 0;

typedef HRESULT(APIENTRY *SetVertexShaderConstantF)(IDirect3DDevice9*, UINT, const float*, UINT);
HRESULT APIENTRY SetVertexShaderConstantF_hook(IDirect3DDevice9*, UINT, const float*, UINT);
SetVertexShaderConstantF SetVertexShaderConstantF_orig = 0;

typedef HRESULT(APIENTRY *DrawIndexedPrimitive)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
HRESULT APIENTRY DrawIndexedPrimitive_hook(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
DrawIndexedPrimitive DrawIndexedPrimitive_orig = 0;

typedef HRESULT(APIENTRY* EndScene) (IDirect3DDevice9*);
HRESULT APIENTRY EndScene_hook(IDirect3DDevice9*);
EndScene EndScene_orig = 0;

typedef HRESULT(APIENTRY *Reset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
HRESULT APIENTRY Reset_hook(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
Reset Reset_orig = 0;

typedef HRESULT(APIENTRY *CreateQuery)(IDirect3DDevice9*, D3DQUERYTYPE, IDirect3DQuery9**);
HRESULT APIENTRY CreateQuery_hook(IDirect3DDevice9*, D3DQUERYTYPE, IDirect3DQuery9**);
CreateQuery CreateQuery_orig = 0;

typedef HRESULT(APIENTRY *SetViewport)(IDirect3DDevice9*, CONST D3DVIEWPORT9*);
HRESULT APIENTRY SetViewport_hook(IDirect3DDevice9*, CONST D3DVIEWPORT9*);
SetViewport SetViewport_orig = 0;

typedef HRESULT(APIENTRY *SetVertexShader)(IDirect3DDevice9*, IDirect3DVertexShader9*);
HRESULT APIENTRY SetVertexShader_hook(IDirect3DDevice9*, IDirect3DVertexShader9*);
SetVertexShader SetVertexShader_orig = 0;

typedef HRESULT(APIENTRY *SetPixelShader)(IDirect3DDevice9*, IDirect3DPixelShader9*);
HRESULT APIENTRY SetPixelShader_hook(IDirect3DDevice9*, IDirect3DPixelShader9*);
SetPixelShader SetPixelShader_orig = 0;

typedef HRESULT(APIENTRY *SetTexture)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*);
HRESULT APIENTRY SetTexture_hook(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*);
SetTexture SetTexture_orig = 0;

//==========================================================================================================================

HRESULT APIENTRY SetStreamSource_hook(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride)
{
	if (StreamNumber == 0)
	{
		Stride = sStride;

		//if (Stride == 12 && pStreamData)
		//{
			//pStreamData->GetDesc(&vdesc);
		//}
	}

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

//=====================================================================================================================

HRESULT APIENTRY SetVertexShaderConstantF_hook(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	if (pConstantData != NULL)
	{
		//pConstantDataFloat = (float*)pConstantData;

		mStartRegister = StartRegister;
		mVector4fCount = Vector4fCount;
	}

	return SetVertexShaderConstantF_orig(pDevice, StartRegister, pConstantData, Vector4fCount);
}

//==========================================================================================================================

HRESULT APIENTRY SetVertexDeclaration_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pDecl)
{
	if (pDecl != NULL)
	{
		HRESULT hr = pDecl->GetDeclaration(decl, &numElements);
		if (FAILED(hr))
		{
			//Log("GetDeclaration failed");
		}
	}

	return SetVertexDeclaration_orig(pDevice, pDecl);
}

//==========================================================================================================================

HRESULT APIENTRY DrawIndexedPrimitive_hook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	//more fps but could cause problems
	if(mStage == 1)
	return DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

	//wallmhack
	if (wallmhack > 0 && decl->Type == 5 && numElements == 11) //models
	{
		//pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);//detected?
		pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

		//chams
		if (wallmhack == 3 && pSize != 164)
		{
			float sRed[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			pDevice->SetPixelShaderConstantF(0, sRed, 1);//0, 4, 8
		}

		//glow
		if (wallmhack == 2 && pSize == 136 && mStartRegister == 235) //hax shader (does not work for everyone)
		{
			pDevice->SetPixelShader(NULL);
		}

		DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

		if (wallmhack == 3 && pSize != 164)
		{
			float sGreen[4] = { 0.0f, 0.5f, 0.0f, 0.0f };
			pDevice->SetPixelShaderConstantF(0, sGreen, 1);//0, 4, 8
		}

		//pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);//detected?
		pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}

	//pause aimbot after killing a target
	if (usehumanaim > 0 && aim_pause > 0 && dWidth == 144 && dHeight == 176 && Stride == 8 && NumVertices == 8 && primCount == 10)
		after_kill = true;

	//smooth aim while aiming
	if (
		(usehumanaim > 0 && Stride == 12 && NumVertices == 192 && primCount == 240) ||
		(usehumanaim > 0 && dWidth == 136 && dHeight == 164 && Stride == 8 && NumVertices == 8 && primCount == 10) ||
		(usehumanaim > 0 && dWidth == 164 && dHeight == 204 && Stride == 8 && NumVertices == 8 && primCount == 10)
		)
		smooth_on = true;

	//killsounds
	if (killsounds == 1 && dWidth == 144 && dHeight == 176 && Stride == 8 && NumVertices == 8 && primCount == 10 && decl->Type == 8 && numElements == 3 && vSize == 476 && pSize == 416 && mStartRegister == 8 && mVector4fCount == 2)
		sound = true;

	//aimbot
	if(
		(aimbot == 1 && pCRC == 0xcb415efe && dWidth == 12 && dHeight == 8 && Stride == 12 && NumVertices == 68 && primCount == 84)|| //orange far range
		(aimbot == 1 && pCRC == 0xcb415efe && dWidth == 12 && dHeight == 8 && Stride == 12 && NumVertices == 66 && primCount == 84)|| //orange mid range
		(aimbot == 1 && pCRC == 0xd81bd7af && dWidth == 12 && dHeight == 8 && Stride == 12 && NumVertices == 66 && primCount == 84) //red near range
		//(aimbot == 1 && pCRC == 0x337194bd && dWidth == 1024 && dHeight == 1024 && dFormat == 50 && Stride == 12 && NumVertices == 66 && primCount == 84)//red/orange near/mid range 5+kills (no)
		)
		HPBarAim(pDevice, 1);

	//aimbot2
	if ((aimbot == 2 && Stride == 8 && NumVertices == 8 && primCount == 10) && (pCRC == 0xd81bd7af || pCRC == 0xcb415efe)) //team red/orange, enemy team
		TBarAim(pDevice, 1);

	//compatibility
	if (aimbot == 3 && NumVertices != 96 && Stride == 8 && NumVertices == 8 && primCount == 10)//all teams
		TBarAim(pDevice, 1);

	//esp
	if (
		(esp == 1 && pCRC == 0xcb415efe && dWidth == 12 && dHeight == 8 && Stride == 12 && NumVertices == 68 && primCount == 84) || //orange far range
		(esp == 1 && pCRC == 0xcb415efe && dWidth == 12 && dHeight == 8 && Stride == 12 && NumVertices == 66 && primCount == 84) || //orange mid range
		(esp == 1 && pCRC == 0xd81bd7af && dWidth == 12 && dHeight == 8 && Stride == 12 && NumVertices == 66 && primCount == 84) //red near range
		)
		HPBarEsp(pDevice, 1);

	//remove texture, used for testing
	//if(NumVertices != 96 && Stride == 12 && primCount == 84 && decl->Type == 4 && numElements == 4 && vSize == 520 && pSize == 360 && mStartRegister == 6 && mVector4fCount == 12)
		//return D3D_OK; 	
	
	/*
	//small bruteforce logger
	if (logger)
	{
		//log hp bar crc with f10
		//if ((Stride == 8 && NumVertices == 8 && primCount == 10 && decl->Type == 8 && numElements == 3 && vSize == 476 && pSize == 416 && mStartRegister == 8 && mVector4fCount == 2) && (GetAsyncKeyState(VK_F10) & 1))
			//Log("pCRC == %x && dWidth == %d && dHeight == %d && dFormat == %d && Stride == %d && NumVertices == %d && primCount == %d && decl->Type == %d && numElements == %d && vSize == %d && pSize == %d && mStartRegister == %d && mVector4fCount == %d", pCRC, dWidth, dHeight, dFormat, Stride, NumVertices, primCount, decl->Type, numElements, vSize, pSize, mStartRegister, mVector4fCount);

		//hold down P key until a texture changes, press I to log values of those textures
		if (GetAsyncKeyState('O') & 1) //-
			countnum--;
		if (GetAsyncKeyState('P') & 1) //+
			countnum++;
		if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState('9') & 1)) //reset, set to -1
			countnum = -1;
		if (countnum == decl->Type)
			if ((Stride > NULL) && (GetAsyncKeyState('I') & 1)) //press I to log to log.txt
				Log("pCRC == %x && dWidth == %d && dHeight == %d && dFormat == %d && Stride == %d && NumVertices == %d && primCount == %d && decl->Type == %d && numElements == %d && vSize == %d && pSize == %d && mStartRegister == %d && mVector4fCount == %d", pCRC, dWidth, dHeight, dFormat, Stride, NumVertices, primCount, decl->Type, numElements, vSize, pSize, mStartRegister, mVector4fCount);
		if (countnum == decl->Type)
		{
			//pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			//DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
			//pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			pDevice->SetPixelShader(NULL);
			return D3D_OK; //delete texture
		}
	}
	*/

	return DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

//==========================================================================================================================

bool DoInit = true;
HRESULT APIENTRY EndScene_hook(IDirect3DDevice9* pDevice)
{
	if (pDevice == nullptr) return EndScene_orig(pDevice);

	//sprite
	PrePalClear(pDevice);

	if (DoInit)
	{
		LoadCfg();

		//wallmhack = Load("wallmhack", "wallmhack", wallmhack, GetDirectoryFile("palaconfig.ini"));

		DoInit = false;
	}

	if (palFont == NULL)
	{
		HRESULT hr = D3DXCreateFont(pDevice, 14, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &palFont);

		if (FAILED(hr)) {
			//Log("D3DXCreateFont failed");
		}
	}

	if (palFont)
	{
		DrawPalMenu(pDevice);
	}

	//smooth after kill
	if (usehumanaim > 0 && after_kill)
	{
		//aimsens = slowsens;  //slow sens
		aimheight = 8; // body height

		if (timeGetTime() - frameafterkill >= 888) //wait  sec
		{
			after_kill = false;
			smooth_on = true;
			frameafterkill = timeGetTime();
		}
	}

	//initial smooth
	if (usehumanaim > 0 && smooth_on && !after_kill)
	{
		//aimsens = fastsens;   //fast sens
		aimheight = 3; //head "lock-on" height

		if ((!GetAsyncKeyState(Daimkey)) || (timeGetTime() - framesmooth >= 444)) //wait sec
		{
			smooth_on = false;
			//aimsens = slowsens;    //slow sens
			aimheight = 8;  //body height
			framesmooth = timeGetTime();
		}

	}

	//prevent tapfire aimlock (add smooth after releasing aimkey)
	if (usehumanaim > 0 && !GetAsyncKeyState(Daimkey))
	{
		after_kill = true;
	}

	//killsounds
	if (sound)
	{
		if (timeGetTime() - soundpause >= 20)
		{
			int random = rand() % 36;

			if (random == 0)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\assasin.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 1)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\bullseye.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 2)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\dominating.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 3)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\doublekill.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 4)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\eagleeye.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 5)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\excellent.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 6)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\godlike.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 7)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\hattrick.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 8)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\headhunter.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 9)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\headshot.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 10)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\impressive.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 11)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\killingmachine.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 12)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\killingspree.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 13)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\maniac.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 14)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\massacre.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 15)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\megakill.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 16)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\monsterkill.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 17)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\multikill.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 18)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\outstanding.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 19)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\payback.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 20)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\rampage.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 21)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\retribution.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 22)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\ultrakill.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 23)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\unreal.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 24)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\unstoppable.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 25)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\vengeance.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 26)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\cheer\\cheer (1).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 27)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (2).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 28)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (3).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 29)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (4).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 30)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (5).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 31)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (6).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 32)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (7).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 33)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (8).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 34)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (9).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 35)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (01).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 36)
				PlaySoundA(GetPalDirFile("stuff\\sounds\\speech\\cheer (11).wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			sound = false;
			soundpause = timeGetTime();
		}
	}

	//draw esp
	if (esp == 1 && EspHPBarInfo.size() != NULL)
	{
		for (unsigned int i = 0; i < EspHPBarInfo.size(); i++)
		{
			//esp
			if(EspHPBarInfo[i].vOutX > 1 && EspHPBarInfo[i].vOutY > 1)
			PrePalPresent2(pDevice, (int)EspHPBarInfo[i].vOutX - 32, (int)EspHPBarInfo[i].vOutY - 20);
			//DrawString(palFont, (int)EspHPBarInfo[i].vOutX, (int)EspHPBarInfo[i].vOutY, D3DCOLOR_ARGB(255, 255, 255, 255), "O");
		}
	}
	EspHPBarInfo.clear();

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

	//do aim 1 (aimbot 1)
	if (aimbot == 1 && AimHPBarInfo.size() != NULL && GetAsyncKeyState(Daimkey) & 0x8000)
	//if (aimbot == 1 && AimHPBarInfo.size() != NULL && GetAsyncKeyState(Daimkey))
	//if (aimbot == 1 && AimHPBarInfo.size() != NULL)
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;

		for (unsigned int i = 0; i < AimHPBarInfo.size(); i++)
		{
			//test w2s
			//if (logger)
			//DrawString(palFont, (int)AimHPBarInfo[i].vOutX, (int)AimHPBarInfo[i].vOutY, D3DCOLOR_ARGB(255, 255, 255, 255), "O");

			//aimfov
			float radiusx = (aimfov*10.0f) * (ScreenCX / 100); 
			float radiusy = (aimfov*10.0f) * (ScreenCY / 100); 

			if (aimfov == 0)
			{
				radiusx = 5.0f * (ScreenCX / 100);
				radiusy = 5.0f * (ScreenCY / 100);
			}

			//get crosshairdistance
			AimHPBarInfo[i].CrosshairDst = GetmDst(AimHPBarInfo[i].vOutX, AimHPBarInfo[i].vOutY, ScreenCX, ScreenCY);

			//aim at team 1 or 2 (not needed)
			//if (aimbot == AimHPBarInfo[i].iTeam)

			//if in fov
			if (AimHPBarInfo[i].vOutX >= ScreenCX - radiusx && AimHPBarInfo[i].vOutX <= ScreenCX + radiusx && AimHPBarInfo[i].vOutY >= ScreenCY - radiusy && AimHPBarInfo[i].vOutY <= ScreenCY + radiusy)

				//get closest/nearest target to crosshair
				if (AimHPBarInfo[i].CrosshairDst < fClosestPos)
				{
					fClosestPos = AimHPBarInfo[i].CrosshairDst;
					BestTarget = i;
				}
		}


		//if nearest target to crosshair
		if (BestTarget != -1)
		{
			double DistX = AimHPBarInfo[BestTarget].vOutX - ScreenCX;
			double DistY = AimHPBarInfo[BestTarget].vOutY - ScreenCY;

			//smooth aim
			DistX /= (1 + aimsens);
			DistY /= (1 + aimsens);

			if (usehumanaim > 0 && smooth_on && !after_kill)
			{
				//smooth aim
				DistX /= (1 + usehumanaim * 2);
				DistY /= (1 + usehumanaim * 2);
			}
			else if (usehumanaim > 0 && after_kill)
			{
				//smooth aim
				DistX /= (1 + usehumanaim + 16 * 2);
				DistY /= (1 + usehumanaim + 16 * 2);
			}
			else
			{
				DistX /= (1 + aimsens * 2);
				DistY /= (1 + aimsens * 2);
			}

			//aim
			//if (GetAsyncKeyState(Daimkey) & 0x8000)
				mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY, 0, NULL);
				//mmousemove((float)DistX, (float)DistY);

			//autoshoot on
			if ((!GetAsyncKeyState(VK_LBUTTON) && (autoshoot == 1))) //
			//if ((!GetAsyncKeyState(VK_LBUTTON) && (autoshoot == 1) && (GetAsyncKeyState(Daimkey) & 0x8000))) //
			{
				if (autoshoot == 1 && !IsPressd)
				{
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					//LLeftClickDown();
					IsPressd = true;
				}
			}
		}
	}
	AimHPBarInfo.clear();


	//do aim 2 (aimbot 2)
	if (aimbot >= 2 && AimTBarInfo.size() != NULL && GetAsyncKeyState(Daimkey) & 0x8000)
	//if (aimbot >= 2 && AimTBarInfo.size() != NULL && GetAsyncKeyState(Daimkey))
	//if (aimbot > 0 && AimTBarInfo.size() != NULL)
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;

		for (unsigned int i = 0; i < AimTBarInfo.size(); i++)
		{
			//test w2s
			//if (logger)
			//DrawStrin(palFont, (int)AimTBarInfo[i].vOutX, (int)AimTBarInfo[i].vOutY, D3DCOLOR_ARGB(255, 0, 255, 0), "o");
			//DrawStrin(palFont, (int)AimTBarInfo[i].vOutX, (int)AimTBarInfo[i].vOutY, Green, "%.f", (float)Daimkey);

			//aimfov
			float radiusx = (aimfov*20.0f) * (ScreenCX / 100); 
			float radiusy = (aimfov*20.0f) * (ScreenCY / 100); 

			if (aimfov == 0)
			{
				radiusx = 10.0f * (ScreenCX / 100);
				radiusy = 10.0f * (ScreenCY / 100);
			}

			//get crosshairdistance
			AimTBarInfo[i].CrosshairDst = GetmDst(AimTBarInfo[i].vOutX, AimTBarInfo[i].vOutY, ScreenCX, ScreenCY);

			//aim at team 1 or 2 (not needed)
			//if (aimbot == AimTBarInfo[i].iTeam)

			//if in fov
			if (AimTBarInfo[i].vOutX >= ScreenCX - radiusx && AimTBarInfo[i].vOutX <= ScreenCX + radiusx && AimTBarInfo[i].vOutY >= ScreenCY - radiusy && AimTBarInfo[i].vOutY <= ScreenCY + radiusy)

				//get closest/nearest target to crosshair
				if (AimTBarInfo[i].CrosshairDst < fClosestPos)
				{
					fClosestPos = AimTBarInfo[i].CrosshairDst;
					BestTarget = i;
				}
		}


		//if nearest target to crosshair
		if (BestTarget != -1)
		{
			double DistX = AimTBarInfo[BestTarget].vOutX - ScreenCX;
			double DistY = AimTBarInfo[BestTarget].vOutY - ScreenCY;

			if (usehumanaim > 0 && smooth_on && !after_kill)
			{
				//smooth aim
				DistX /= (1 + usehumanaim *2);
				DistY /= (1 + usehumanaim *2);
			}
			else if (usehumanaim > 0 && after_kill)
			{
				//smooth aim
				DistX /= (1 + usehumanaim + 16 * 2);
				DistY /= (1 + usehumanaim + 16 * 2);
			}
			else
			{
				DistX /= (1 + aimsens * 2);
				DistY /= (1 + aimsens * 2);
			}

			//aim
			//if ((botpause==false) && (GetAsyncKeyState(Daimkey) & 0x8000))
				mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY, 0, NULL);
				//mmousemove((float)DistX, (float)DistY);

			//autoshoot on
			if ((!GetAsyncKeyState(VK_LBUTTON) && (autoshoot == 1))) //
			//if ((!GetAsyncKeyState(VK_LBUTTON) && (autoshoot == 1) && (GetAsyncKeyState(Daimkey) & 0x8000))) //
			{
				if (autoshoot == 1 && !IsPressd)
				{
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					//LLeftClickDown();
					IsPressd = true;
				}
			}
		}
	}
	AimTBarInfo.clear();


	//autoshoot off
	if (autoshoot == 1 && IsPressd)
	{
		if (timeGetTime() - astime >= asdelay)
		{
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//LLeftClickUp();
			IsPressd = false;
			astime = timeGetTime();
		}
	}

	/*
	//draw logger
	if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState(VK_CONTROL)) && (GetAsyncKeyState(0x4C) & 1)) //ALT + CTRL + L toggles logger
		logger = !logger;
	if (palFont && logger) //&& countnum >= 0)
	{
		char szString[255];
		sprintf_s(szString, "countnum = %d", countnum);
		DrawStrin(palFont, 220, 100, White, (char*)&szString[0]);
		DrawStrin(palFont, 220, 110, Yellow, "hold P to +");
		DrawStrin(palFont, 220, 120, Yellow, "hold O to -");
		DrawStrin(palFont, 220, 130, Green, "press I to log");
	}
	*/

	return EndScene_orig(pDevice);
}

//==========================================================================================================================

class WinQuery1338343904 : public IDirect3DQuery9
{
public:
	HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObj)
	{
		return D3D_OK;
	}

	ULONG WINAPI AddRef()
	{
		mref++;

		return mref;
	}

	ULONG WINAPI Release()
	{
		return 1;
	}

	HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice)
	{
		return D3D_OK;
	}

	D3DQUERYTYPE WINAPI GetType()
	{
		return mtype;
	}

	DWORD WINAPI GetDataSize()
	{
		return sizeof(DWORD);
	}

	HRESULT    WINAPI Issue(DWORD dwIssueFlags)
	{
		return D3D_OK;
	}

	HRESULT WINAPI GetData(void* pData, DWORD dwSize, DWORD dwGetDataFlags)
	{
		DWORD* winData = (DWORD*)pData;

		*winData = 500; //500 pixels visible
		return D3D_OK;
	}

	int                mref;
	D3DQUERYTYPE    mtype;
};

HRESULT APIENTRY CreateQuery_hook(IDirect3DDevice9* pDevice, D3DQUERYTYPE Type, IDirect3DQuery9** pQuery)
{

	//Anti-occlusion v2 (used by wallhack to see models through walls at all distances, reduces fps, on/off requires vid_restart F11 or alt + enter)
	if(occlusion == 1 && Type == D3DQUERYTYPE_OCCLUSION)
	{
		*pQuery = new WinQuery1338343904;

		((WinQuery1338343904*)*pQuery)->mtype = Type;

		return D3D_OK;
	}

	//if (Type == D3DQUERYTYPE_OCCLUSION)
	//{
		//Type = D3DQUERYTYPE_EVENT;// D3DQUERYTYPE_TIMESTAMP;
	//}

	return CreateQuery_orig(pDevice, Type, pQuery);
}

//==========================================================================================================================

HRESULT APIENTRY SetViewport_hook(IDirect3DDevice9* pDevice, CONST D3DVIEWPORT9* pViewport)
{
	//get viewport/screensize
	Viewport = *pViewport;
	ScreenCX = (float)Viewport.Width / 2.0f;
	ScreenCY = (float)Viewport.Height / 2.0f;

	return SetViewport_orig(pDevice, pViewport);
}

//==========================================================================================================================

HRESULT APIENTRY Reset_hook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	DeleteSurfaces();

	if (palFont)
		palFont->OnLostDevice();

	HRESULT ResetReturn = Reset_orig(pDevice, pPresentationParameters);
	
	if (SUCCEEDED(ResetReturn))
	{
		if (palFont)
			palFont->OnResetDevice();
	}

	return ResetReturn;
}

//==========================================================================================================================

HRESULT APIENTRY SetVertexShader_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9 *veShader)
{
	if (veShader != NULL)
	{
		vShader = veShader;
		vShader->GetFunction(NULL, &vSize);
	}
	return SetVertexShader_orig(pDevice, veShader);
}

//==========================================================================================================================

HRESULT APIENTRY SetPixelShader_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9 *piShader)
{
	if (piShader != NULL)
	{
		pShader = piShader;
		pShader->GetFunction(NULL, &pSize);
	}
	return SetPixelShader_orig(pDevice, piShader);
}

//==========================================================================================================================

HRESULT APIENTRY SetTexture_hook(IDirect3DDevice9* pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	if (pDevice == nullptr) return SetTexture_orig(pDevice, Sampler, pTexture);

	mStage = Sampler;

	if((aimbot == 1||aimbot == 2)&&(decl->Type == 8 && numElements == 3 && mStartRegister == 6 && mVector4fCount == 2 && Sampler == 0 && pTexture))//vSize == 476 && pSize == 416 //reduce fps loss
	{//1
		pCurrentTex = static_cast<IDirect3DTexture9*>(pTexture);

		//if (pCurrentTex)//
		//{
			D3DSURFACE_DESC surfaceDesc;

			//pCurrentTex->GetLevelDesc(0, &surfaceDesc);
			if (FAILED(pCurrentTex->GetLevelDesc(0, &surfaceDesc)))
			{
				//Log("surfaceDesc failed");
				goto out;
			}

			if (SUCCEEDED(pCurrentTex->GetLevelDesc(0, &surfaceDesc)))//can crash after game shuts down
			if (surfaceDesc.Pool == D3DPOOL_MANAGED && pCurrentTex->GetType() == D3DRTYPE_TEXTURE) //reduce fps loss
			{
				dWidth = surfaceDesc.Width;
				dHeight = surfaceDesc.Height;
				//dFormat = surfaceDesc.Format;

				//if (pCurrentTex->GetType() == D3DRTYPE_TEXTURE)
				//if (reinterpret_cast<IDirect3DTexture9 *>(pCurrentTex)->GetType() == D3DRTYPE_TEXTURE)
				//if ((surfaceDesc.Width == 12 && surfaceDesc.Height == 8 && surfaceDesc.Format == 894720068 || surfaceDesc.Width == 1024 && surfaceDesc.Height == 1024 && surfaceDesc.Format == 50) && (pCurrentTex->GetType() == D3DRTYPE_TEXTURE))//
				//{
					D3DLOCKED_RECT pLockedRect;

					if (pCurrentTex->LockRect(0, &pLockedRect, NULL, D3DLOCK_READONLY | D3DLOCK_DONOTWAIT | D3DLOCK_NOSYSLOCK) == S_OK)
					//pCurrentTex->LockRect(0, &pLockedRect, NULL, D3DLOCK_NOOVERWRITE | D3DLOCK_READONLY);
					//pCurrentTex->LockRect(0, &pLockedRect, NULL, D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK | D3DLOCK_DONOTWAIT | D3DLOCK_READONLY);

					//if (&pLockedRect != NULL && pLockedRect.pBits != NULL && pLockedRect.Pitch != NULL)
					if (pLockedRect.pBits != NULL)
					{
						// get crc from the algorithm
						pCRC = QuickPChecksm((DWORD*)pLockedRect.pBits, pLockedRect.Pitch);
						pCurrentTex->UnlockRect(0);
						//pCurrentTex->Release();
					}
					//pCurrentTex->UnlockRect(0);
				//}
			}
		//}
	}
	out:
	
	return SetTexture_orig(pDevice, Sampler, pTexture);
}

//==========================================================================================================================

DWORD WINAPI PalD3D(__in  LPVOID lpParameter)
{
	HWND WindWND = NULL;
	while (!WindWND)
	{
		//Multi national version compatible 
		WindWND = FindWindowA("LaunchUnrealUWindowsClient", 0); //avoid inj i.s.o.g
		Sleep(100);
	}
	CloseHandle(WindWND);

	HMODULE WindDLL = NULL;
	while (!WindDLL)
	{
		WindDLL = GetModuleHandleA("d3d9.dll");
		Sleep(100);
	}
	CloseHandle(WindDLL);

	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;

	HWND tmpWnd = CreateWindowA("BUTTON", "PalaD3D", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, PalHand, NULL);
	if(tmpWnd == NULL)
	{
		//Log("[DirectX] Failed to create temp window");
		return 0;
	}

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if(d3d == NULL)
	{
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D interface");
		return 0;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp)); 
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = tmpWnd;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if(result != D3D_OK)
	{
		d3d->Release();
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D device");
		return 0;
	}

	// We have the device, so walk the vtable to get the address of all the dx functions in d3d9.dll
	#if defined _M_X64
	DWORD64* dVtable = (DWORD64*)d3ddev;
	dVtable = (DWORD64*)dVtable[0];
	#elif defined _M_IX86
	DWORD* dVtable = (DWORD*)d3ddev;
	dVtable = (DWORD*)dVtable[0]; // == *d3ddev
	#endif
	//Log("[DirectX] dvtable: %x", dVtable);

	//for(int i = 0; i < 95; i++)
	//{
		//Log("[DirectX] vtable[%i]: %x, pointer at %x", i, dVtable[i], &dVtable[i]);
	//}

	// Set EndScene_orig to the original EndScene etc.
	EndScene_orig = (EndScene)dVtable[42];
	SetStreamSource_orig = (SetStreamSource)dVtable[100];
	SetVertexDeclaration_orig = (SetVertexDeclaration)dVtable[87];
	SetVertexShaderConstantF_orig = (SetVertexShaderConstantF)dVtable[94];
	DrawIndexedPrimitive_orig = (DrawIndexedPrimitive)dVtable[82];
	Reset_orig = (Reset)dVtable[16];
	CreateQuery_orig = (CreateQuery)dVtable[118];
	SetViewport_orig = (SetViewport)dVtable[47];
	SetVertexShader_orig = (SetVertexShader)dVtable[92];
	SetPixelShader_orig = (SetPixelShader)dVtable[107];
	SetTexture_orig = (SetTexture)dVtable[65];

	// Detour functions x86 & x64
	if (MH_Initialize() != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[42], &EndScene_hook, reinterpret_cast<void**>(&EndScene_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[42]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[100], &SetStreamSource_hook, reinterpret_cast<void**>(&SetStreamSource_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[100]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[87], &SetVertexDeclaration_hook, reinterpret_cast<void**>(&SetVertexDeclaration_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[87]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[94], &SetVertexShaderConstantF_hook, reinterpret_cast<void**>(&SetVertexShaderConstantF_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[94]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[82], &DrawIndexedPrimitive_hook, reinterpret_cast<void**>(&DrawIndexedPrimitive_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[82]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[16], &Reset_hook, reinterpret_cast<void**>(&Reset_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[16]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[118], &CreateQuery_hook, reinterpret_cast<void**>(&CreateQuery_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[118]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[47], &SetViewport_hook, reinterpret_cast<void**>(&SetViewport_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[47]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[92], &SetVertexShader_hook, reinterpret_cast<void**>(&SetVertexShader_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[92]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[107], &SetPixelShader_hook, reinterpret_cast<void**>(&SetPixelShader_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[107]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[65], &SetTexture_hook, reinterpret_cast<void**>(&SetTexture_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[65]) != MH_OK) { return 1; }

	//Log("[Detours] Detours attached\n");

	d3ddev->Release();
	d3d->Release();
	DestroyWindow(tmpWnd);
		
	return 1;
}

//==========================================================================================================================

BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	//DWORD ModuleBase = (DWORD)hinstDLL;
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: // A process is loading the DLL.
		PalHand = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL); // disable unwanted thread notifications to reduce overhead
		GetModuleFileNameA(hinstDLL, paldir, 512);
		for (int i = (int)strlen(paldir); i > 0; i--)
		{
			if (paldir[i] == '\\')
			{
				paldir[i + 1] = 0;
				break;
			}
		}

		CreateThread(0, 0, PalD3D, 0, 0, 0); //init our hooks
		break;

	case DLL_PROCESS_DETACH: // A process unloads the DLL.
		/*
		if (MH_Uninitialize() != MH_OK) { return 1; }
		if (MH_DisableHook((DWORD_PTR*)dVtable[42]) != MH_OK) { return 1; }
		*/
		break;
	}
	return TRUE;
}
