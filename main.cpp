/*
* Paladins D3D Hack Source V2.0

Requires: eac bypass

Set game to dx9:
Documents\My Games\paladinslive\ChaosGame\Config\ChaosSystemSettings.ini UseDX11=False
Documents\My Games\paladinspts\ChaosGame\Config\ChaosSystemSettings.ini UseDX11=False

How to compile:
- compile with visual studio community 2017 (..\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.exe)
- select Release x86 or x64 for paladins battlegrounds version
- click: project -> properties -> configuration properties -> general -> character set -> change to "not set"

Optional: remove dependecy on vs runtime:
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

typedef HRESULT(APIENTRY *SetVertexDeclaration)(IDirect3DDevice9*, IDirect3DVertexDeclaration9*);
HRESULT APIENTRY SetVertexDeclaration_hook(IDirect3DDevice9*, IDirect3DVertexDeclaration9*);
SetVertexDeclaration SetVertexDeclaration_orig = 0;

typedef HRESULT(APIENTRY *SetStreamSource)(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
HRESULT APIENTRY SetStreamSource_hook(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
SetStreamSource SetStreamSource_orig = 0;

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

typedef HRESULT(APIENTRY *SetView__port)(IDirect3DDevice9*, CONST D3DVIEWPORT9*);
HRESULT APIENTRY SetView__port_hook(IDirect3DDevice9*, CONST D3DVIEWPORT9*);
SetView__port SetView__port_orig = 0;

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

HRESULT APIENTRY SetVertexDeclaration_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pdecl_)
{
	if (pdecl_ != NULL)
	{
		HRESULT hr = pdecl_->GetDeclaration(decl_, &num__elements);
		if (FAILED(hr))
		{
			//Log("Getdecl_aration failed");
		}
	}

	return SetVertexDeclaration_orig(pDevice, pdecl_);
}

//==========================================================================================================================

HRESULT APIENTRY SetStreamSource_hook(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride_)
{
	if (StreamNumber == 0)
	{
		Stride_ = sStride_;

		//if (pStreamData)
		//{
			//pStreamData->GetDesc(&vdesc);
		//}
	}

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride_);
}

//=====================================================================================================================

HRESULT APIENTRY SetVertexShaderConstantF_hook(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	if (pConstantData != NULL)
	{
		//pConstantDataFloat = (float*)pConstantData;

		mStart__register = StartRegister;
		mVector__Count = Vector4fCount;
	}

	return SetVertexShaderConstantF_orig(pDevice, StartRegister, pConstantData, Vector4fCount);
}

//==========================================================================================================================

//big enemy hp bars
//dWidth_ == 12 && dHeight_ == 8 && Stride_ == 12 && NumVertices == 68 && primCount == 84 && decl_->Type == 4 && num__elements == 4 && vSize_ == 520 && pSize_ == 360 && mStart__register == 6 && mVector__Count == 12 
//dWidth_ == 12 && dHeight_ == 8 && Stride_ == 12 && NumVertices == 66 && primCount == 84 && decl_->Type == 4 && num__elements == 4 && vSize_ == 520 && pSize_ == 360 && mStart__register == 6 && mVector__Count == 12 
//dWidth_ == 12 && dHeight_ == 8 && Stride_ == 12 && NumVertices == 66 && primCount == 84 && decl_->Type == 4 && num__elements == 4 && vSize_ == 520 && pSize_ == 360 && mStart__register == 6 && mVector__Count == 12 

//small t
//dWidth_ == 12 && dHeight_ == 8 && Stride_ == 8 && NumVertices == 8 && primCount == 10 && decl_->Type == 8 && num__elements == 3 && vSize_ == 476 && pSize_ == 416 && mStart__register == 8 && mVector__Count == 2 
//dWidth_ == 12 && dHeight_ == 8 && Stride_ == 8 && NumVertices == 8 && primCount == 10 && decl_->Type == 8 && num__elements == 3 && vSize_ == 476 && pSize_ == 416 && mStart__register == 8 && mVector__Count == 2 

HRESULT APIENTRY DrawIndexedPrimitive_hook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	//more fps but could cause problems with wallhack at lower setting
	if (wall__hack == 0 && mStage_ == 1)
		return DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

	//wallhack
	if (wall__hack > 0 && decl_->Type == 5 && num__elements == 11) //models
	{
		//wallhack
		pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		//pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

		//chams fix me
		if (wall__hack == 3 && pSize_ != 164)
		{
			float sRed_[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			pDevice->SetPixelShaderConstantF(0, sRed_, 1);
		}

		//glow
		if (wall__hack == 2 && pSize_ == 136 && mStart__register == 235) //hax shader (does not work for everyone)
		{
			pDevice->SetPixelShader(NULL);
		}

		DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

		//chams
		if (wall__hack == 3 && pSize_ != 164)
		{
			float sGreen_[4] = { 0.0f, 0.5f, 0.0f, 0.0f };
			pDevice->SetPixelShaderConstantF(0, sGreen_, 1);
		}

		//wallhack
		pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		//pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}

	//pause aimbot after killing a target
	if (useaim__human > 0 && aim__pause > 0 && dWidth_ == 144 && dHeight_ == 176 && Stride_ == 8 && NumVertices == 8 && primCount == 10)
		after__kill = true;

	//smooth aim while aiming
	if (
		(useaim__human > 0 && Stride_ == 12 && NumVertices == 192 && primCount == 240) ||
		(useaim__human > 0 && dWidth_ == 136 && dHeight_ == 164 && Stride_ == 8 && NumVertices == 8 && primCount == 10) ||
		(useaim__human > 0 && dWidth_ == 164 && dHeight_ == 204 && Stride_ == 8 && NumVertices == 8 && primCount == 10)
		)
		smooth__on = true;

	//kinessa max charge
	if (snipermode == 1 && Stride_ == 12 && NumVertices == 64 && primCount == 80 && pSize_ == 360 && mVector__Count == 2)
		maxcharge = true;

	//kill sounds
	if (kill__sounds == 1 && dWidth_ == 144 && dHeight_ == 176 && Stride_ == 8 && NumVertices == 8 && primCount == 10 && decl_->Type == 8 && num__elements == 3 && /*vSize_ == 476 && */pSize_ == 416 && mStart__register == 8 && mVector__Count == 2)
		sound_ = true;

	//aimbot 1
	if(
		(aim__bot == 1 && CRC == 0x9df78443 && dWidth_ == 12 && dHeight_ == 8 && Stride_ == 12 && NumVertices == 68 && primCount == 84)|| //orange far range
		(aim__bot == 1 && CRC == 0x9df78443 && dWidth_ == 12 && dHeight_ == 8 && Stride_ == 12 && NumVertices == 66 && primCount == 84)|| //orange mid range
		(aim__bot == 1 && CRC == 0xd81bd7af && dWidth_ == 12 && dHeight_ == 8 && Stride_ == 12 && NumVertices == 66 && primCount == 84)	//red near range
		)
		HPBarAim_(pDevice, 8);//8 = w2sStartRegister

	//aimbot 2
	if ((aim__bot == 2 && dWidth_ == 12 && dHeight_ == 8 && Stride_ == 8 && NumVertices == 8 && primCount == 10) && (CRC == 0xd81bd7af || CRC == 0x9df78443)) //team red/orange, enemy team
		//TBarAim(pDevice, 6);//6 = w2sStartRegister
		HPBarAim_(pDevice, 6);

	
	//small bruteforce logger
	if (logger)
	{
		//log hp bars (warning: we did not hook setvertexshader)
		if ((Stride_ == 8 && NumVertices == 8 && primCount == 10 && decl_->Type == 8 && num__elements == 3 && vSize_ == 476 && pSize_ == 416 && mStart__register == 8 && mVector__Count == 2) && (GetAsyncKeyState(VK_F9) & 1))
			Log("small CRC == %x && dWidth_ == %d && dHeight_ == %d && Stride_ == %d && NumVertices == %d && primCount == %d && decl_->Type == %d && num__elements == %d && vSize_ == %d && pSize_ == %d && mStart__register == %d && mVector__Count == %d", CRC, dWidth_, dHeight_, Stride_, NumVertices, primCount, decl_->Type, num__elements, vSize_, pSize_, mStart__register, mVector__Count);
		if ((dWidth_ == 12 && dHeight_ == 8 && Stride_ == 12 && NumVertices == 68 && primCount == 84) && (GetAsyncKeyState(VK_F10) & 1))
			Log("big CRC == %x && dWidth_ == %d && dHeight_ == %d && Stride_ == %d && NumVertices == %d && primCount == %d && decl_->Type == %d && num__elements == %d && vSize_ == %d && pSize_ == %d && mStart__register == %d && mVector__Count == %d", CRC, dWidth_, dHeight_, Stride_, NumVertices, primCount, decl_->Type, num__elements, vSize_, pSize_, mStart__register, mVector__Count);
		if ((dWidth_ == 12 && dHeight_ == 8 && Stride_ == 12 && NumVertices == 66 && primCount == 84) && (GetAsyncKeyState(VK_F10) & 1))
			Log("big CRC == %x && dWidth_ == %d && dHeight_ == %d && Stride_ == %d && NumVertices == %d && primCount == %d && decl_->Type == %d && num__elements == %d && vSize_ == %d && pSize_ == %d && mStart__register == %d && mVector__Count == %d", CRC, dWidth_, dHeight_, Stride_, NumVertices, primCount, decl_->Type, num__elements, vSize_, pSize_, mStart__register, mVector__Count);

		//hold down P key until a texture changes, press I to log values of those textures
		if (GetAsyncKeyState('O') & 1) //-
			countnum--;
		if (GetAsyncKeyState('P') & 1) //+
			countnum++;
		if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState('9') & 1)) //reset, set to -1
			countnum = -1;
		if (countnum == NumVertices)
			if ((Stride_ > NULL) && (GetAsyncKeyState('I') & 1)) //press I to log to log.txt
				Log("CRC == %x && dWidth_ == %d && dHeight_ == %d && Stride_ == %d && NumVertices == %d && primCount == %d && decl_->Type == %d && num__elements == %d && vSize_ == %d && pSize_ == %d && mStart__register == %d && mVector__Count == %d", CRC, dWidth_, dHeight_, Stride_, NumVertices, primCount, decl_->Type, num__elements, vSize_, pSize_, mStart__register, mVector__Count);
		if (countnum == NumVertices)
		{
			//pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			//DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
			//pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			//pDevice->SetPixelShader(NULL);
			return D3D_OK; //delete texture
		}
	}
	
	return DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

//==========================================================================================================================

HRESULT APIENTRY EndScene_hook(IDirect3DDevice9* pDevice)
{
	//if (pDevice == nullptr) return EndScene_orig(pDevice);

	//sprite
//	PreClear(pDevice);

	if (FirstInit)
	{
		FirstInit = false;
		PlaySoundA(GetDirFile("snd\\smileonmyface5.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);
	}

	if (DoInit)
	{
		DoInit = false;

		//get Viewport/screensize
		pDevice->GetViewport(&View__port);
		ScreenCX = (float)View__port.Width / 2.0f;
		ScreenCY = (float)View__port.Height / 2.0f;

		LoadCfg();

		//wall__hack = Load("wall__hack", "wall__hack", wall__hack, GetDirectoryFile("aconfig.ini"));
	}

	if (Font == NULL)
	{
		HRESULT hr = D3DXCreateFont(pDevice, 14, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Italic"), &Font);

		if (FAILED(hr)) {
			//Log("D3DXCreateFont failed");
		}
	}

	if (Font)
	{
		DrawMenu(pDevice);
	}

	//smooth after kill
	if (useaim__human > 0 && after__kill)
	{
		//aim__sens = slowsens;  //slow sens
		aim__height = 8; // body height

		if (timeGetTime() - farmeafter__kill >= 889) //wait  sec
		{
			after__kill = false;
			smooth__on = true;
			farmeafter__kill = timeGetTime();
		}
	}

	//initial smooth
	if (useaim__human > 0 && smooth__on && !after__kill)
	{
		//aim__sens = fastsens;   //fast sens
		aim__height = 3; //head "lock-on" height

		if ((!GetAsyncKeyState(Daim__key)) || (timeGetTime() - frame__smooth >= 444)) //wait sec
		{
			smooth__on = false;
			//aim__sens = slowsens;    //slow sens
			aim__height = 8;  //body height
			frame__smooth = timeGetTime();
		}

	}

	//prevent tapfire aimlock (add smooth after releasing aim__key)
	if (useaim__human > 0 && !GetAsyncKeyState(Daim__key))
	{
		after__kill = true;
	}

	//kill sounds
	if (sound_)
	{
		if (timeGetTime() - sound__pause >= 20)
		{
			int random = rand() % 17;

			if (random == 0)
				PlaySoundA(GetDirFile("snd\\drum1.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 1)
				PlaySoundA(GetDirFile("snd\\drum2.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 2)
				PlaySoundA(GetDirFile("snd\\drum3.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 3)
				PlaySoundA(GetDirFile("snd\\drum4.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 4)
				PlaySoundA(GetDirFile("snd\\drum5.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 5)
				PlaySoundA(GetDirFile("snd\\drum6.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 6)
				PlaySoundA(GetDirFile("snd\\drum7.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 7)
				PlaySoundA(GetDirFile("snd\\drum8.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 8)
				PlaySoundA(GetDirFile("snd\\hum1.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 9)
				PlaySoundA(GetDirFile("snd\\hum2.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 10)
				PlaySoundA(GetDirFile("snd\\intime1.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 11)
				PlaySoundA(GetDirFile("snd\\tofeel2.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 12)
				PlaySoundA(GetDirFile("snd\\runfromit3.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 13)
				PlaySoundA(GetDirFile("snd\\funisnotsomething4.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 14)
				PlaySoundA(GetDirFile("snd\\smileonmyface5.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 15)
				PlaySoundA(GetDirFile("snd\\end1.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 16)
				PlaySoundA(GetDirFile("snd\\end2.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			if (random == 17)
				PlaySoundA(GetDirFile("snd\\smileonmyface5.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);

			sound_ = false;
			sound__pause = timeGetTime();
		}
	}


	//Shift|RMouse|LMouse|Ctrl|Alt|Space|X|C
	if (aim__key == 0) Daim__key = 0;
	if (aim__key == 1) Daim__key = VK_SHIFT;
	if (aim__key == 2) Daim__key = VK_RBUTTON;
	if (aim__key == 3) Daim__key = VK_LBUTTON;
	if (aim__key == 4) Daim__key = VK_CONTROL;
	if (aim__key == 5) Daim__key = VK_MENU;
	if (aim__key == 6) Daim__key = VK_SPACE;
	if (aim__key == 7) Daim__key = 0x58; //X
	if (aim__key == 8) Daim__key = 0x43; //C

	//do aim 1 (aim__bot 1)
	//if ((aim__bot == 1 && AimHPBar__Info.size() != NULL && GetAsyncKeyState(Daim__key) & 0x8000) || (aim__bot == 2 && AimHPBar__Info.size() != NULL && GetAsyncKeyState(Daim__key) & 0x8000)) //warning: GetAsyncKeyState here will cause aimbot not to work for a few people
	if ((aim__bot == 1 && AimHPBar__Info.size() != NULL)||(aim__bot == 2 && AimHPBar__Info.size() != NULL))
	{
		UINT Best__Target = -1;
		DOUBLE fClosest__Pos = 99999;

		for (unsigned int i = 0; i < AimHPBar__Info.size(); i++)
		{
			//test w2s
			if (logger)
			DrawString(Font, (int)AimHPBar__Info[i].vOutX_, (int)AimHPBar__Info[i].vOutY_, D3DCOLOR_ARGB(255, 255, 255, 255), "O");
			//DrawString(Font, (int)AimHPBar__Info[i].vOutX_, (int)AimHPBar__Info[i].vOutY_, D3DCOLOR_ARGB(255, 255, 255, 255), "%.f", (float)AimHPBar__Info.size());

			//esp
			if(es__p > 0 && AimHPBar__Info[i].vOutX_ > 1 && AimHPBar__Info[i].vOutY_ > 1)
				DrawLine(pDevice, (int)AimHPBar__Info[i].vOutX_, (int)AimHPBar__Info[i].vOutY_, ScreenCX, ScreenCY * ((float)es__p * 0.2f), 20, D3DCOLOR_ARGB(255, 255, 255, 255));//2down,1middle,0up


			//aim__fov
			float radiusx_ = (aim__fov*10.0f) * (ScreenCX / 100.0f); 
			float radiusy_ = (aim__fov*10.0f) * (ScreenCY / 100.0f); 

			if (aim__fov == 0)
			{
				radiusx_ = 5.0f * (ScreenCX / 100.0f);
				radiusy_ = 5.0f * (ScreenCY / 100.0f);
			}

			//get crosshairdistance
			AimHPBar__Info[i].Crosshair__Distance = GetDistance(AimHPBar__Info[i].vOutX_, AimHPBar__Info[i].vOutY_, ScreenCX, ScreenCY);

			//aim at team 1 or 2 (not needed)
			//if (aim__bot == AimHPBar__Info[i].iTeam_)

			//if in fov
			if (AimHPBar__Info[i].vOutX_ >= ScreenCX - radiusx_ && AimHPBar__Info[i].vOutX_ <= ScreenCX + radiusx_ && AimHPBar__Info[i].vOutY_ >= ScreenCY - radiusy_ && AimHPBar__Info[i].vOutY_ <= ScreenCY + radiusy_)

				//get closest/nearest target to crosshair
				if (AimHPBar__Info[i].Crosshair__Distance < fClosest__Pos)
				{
					fClosest__Pos = AimHPBar__Info[i].Crosshair__Distance;
					Best__Target = i;
				}
		}


		//if nearest target to crosshair
		if (Best__Target != -1)
		{
			double DistX_ = AimHPBar__Info[Best__Target].vOutX_ - ScreenCX;
			double DistY_ = AimHPBar__Info[Best__Target].vOutY_ - ScreenCY;

			DistX_ /= (0.5f + (float)aim__sens*0.5f);
			DistY_ /= (0.5f + (float)aim__sens*0.5f);

			if (useaim__human > 0 && /*smooth__on && */!after__kill)
			{
				//smooth aim
				DistX_ /= (1 + useaim__human * 2);
				DistY_ /= (1 + useaim__human * 2);
			}
			if (useaim__human > 0 && after__kill)
			{
				//smooth aim
				DistX_ /= (1 + useaim__human + 16 * 2);
				DistY_ /= (1 + useaim__human + 16 * 2);
			}

			if (useaim__human == 0)
			{
				DistX_ /= (1 + aim__sens);
				DistY_ /= (1 + aim__sens);
			}

			//normal mode aim
			if (snipermode == 0 && GetAsyncKeyState(Daim__key) & 0x8000)
			//if (snipermode == 0)
				mouse_event(MOUSEEVENTF_MOVE, (float)DistX_, (float)DistY_, 0, NULL);

			//sniper aimtrigger
			if (snipermode == 1 && maxcharge == true && GetAsyncKeyState(Daim__key) & 0x8000)
			//if (snipermode == 1 && maxcharge == true)
			{
				mouse_event(MOUSEEVENTF_MOVE, (float)DistX_, (float)DistY_, 0, NULL);
			}

			//sniper autoshoot
			if ((!GetAsyncKeyState(VK_LBUTTON) && (auto__shoot == 1) && (snipermode == 1) && (maxcharge == true) && (GetAsyncKeyState(Daim__key) & 0x8000)))
			//if ((!GetAsyncKeyState(VK_LBUTTON) && (auto__shoot == 1) && (snipermode == 1) && (maxcharge == true)))
			{
				AimHPBar__Info[Best__Target].Crosshair__Distance = GetDistance(AimHPBar__Info[Best__Target].vOutX_, AimHPBar__Info[Best__Target].vOutY_, ScreenCX, ScreenCY);
				if (auto__shoot == 1 && snipermode == 1 && !Is__Pressed && AimHPBar__Info[Best__Target].Crosshair__Distance < triggerdist)
				{
					maxcharge = false;
					//DrawPoint(Device, (int)100, (int)100, 26, 26, 0xFFFF0000);
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					Is__Pressed = true;
					maxcharge = false;
				}
			}

			//autoshoot on
			if ((!GetAsyncKeyState(VK_LBUTTON) && (auto__shoot == 1) && (snipermode == 0) && (GetAsyncKeyState(Daim__key) & 0x8000))) //
			//if ((!GetAsyncKeyState(VK_LBUTTON) && (auto__shoot == 1) && (snipermode == 0))) //
			{
				if (auto__shoot == 1 && !Is__Pressed)
				{
					Is__Pressed = true;
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					//LLeftClickDown();
				}
			}
		}
	}
	AimHPBar__Info.clear();

	
	//auto__shoot off
	if (auto__shoot == 1 && Is__Pressed)
	{
		if (timeGetTime() - astime_ >= asdelay_)
		{
			Is__Pressed = false;
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//LLeftClickUp();
			astime_ = timeGetTime();
		}
	}

	
	//draw logger
	if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState(VK_CONTROL)) && (GetAsyncKeyState(0x4C) & 1)) //ALT + CTRL + L toggles logger
		logger = !logger;
	if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState(0x4C)) && (GetAsyncKeyState(VK_CONTROL) & 1)) //ALT + CTRL + L toggles logger
		logger = !logger;
	if (Font && logger) //&& countnum >= 0)
	{
		char szString[255];
		sprintf_s(szString, "countnum = %d", countnum);
		DrawString(Font, 220, 100, D3DCOLOR_ARGB(255, 255, 255, 255), (char*)&szString[0]);
		DrawString(Font, 220, 110, D3DCOLOR_ARGB(255, 255, 255, 255), "hold P to +");
		DrawString(Font, 220, 120, D3DCOLOR_ARGB(255, 255, 255, 255), "hold O to -");
		DrawString(Font, 220, 130, D3DCOLOR_ARGB(255, 255, 255, 255), "press I to log");
	}
	
	return EndScene_orig(pDevice);
}

//==========================================================================================================================

class CreatQuery : public IDirect3DQuery9
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
		DWORD* Data = (DWORD*)pData;

		*Data = 500; //500 pixels visible
		return D3D_OK;
	}

	int                mref;
	D3DQUERYTYPE    mtype;
};

HRESULT APIENTRY CreateQuery_hook(IDirect3DDevice9* pDevice, D3DQUERYTYPE Type, IDirect3DQuery9** pQuery)
{
	//Anti-occlusion v2 (used by wallhack to see models through walls at all distances, reduces fps, on/off requires vid_restart F11 or alt + enter)
	if(occlu__sion == 1 && Type == D3DQUERYTYPE_OCCLUSION)
	{
		*pQuery = new CreatQuery;

		((CreatQuery*)*pQuery)->mtype = Type;

		return D3D_OK;
	}

	//worse alternative
	//if(occlu__sion == 1 &&Type == D3DQUERYTYPE_OCCLUSION)
	//{
		//Type = D3DQUERYTYPE_EVENT;// D3DQUERYTYPE_TIMESTAMP;
	//}

	return CreateQuery_orig(pDevice, Type, pQuery);
}

//==========================================================================================================================

HRESULT APIENTRY SetView__port_hook(IDirect3DDevice9* pDevice, CONST D3DVIEWPORT9* pView__port)
{
	if (DoInit)
	{
		DoInit = false;

		//get Viewport/screensize
		View__port = *pView__port;
		ScreenCX = (float)View__port.Width / 2.0f;
		ScreenCY = (float)View__port.Height / 2.0f;
	}

	return SetView__port_orig(pDevice, pView__port);
}

//==========================================================================================================================

HRESULT APIENTRY Reset_hook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
//	DeleteSurfaces();

	if (Font)
		Font->OnLostDevice();

	//if (line)
		//line->OnLostDevice();

	//stateOnLostDevice(pDevice);

	HRESULT Reset__Return = Reset_orig(pDevice, pPresentationParameters);
	
	if (SUCCEEDED(Reset__Return))
	{
		if (Font)
			Font->OnResetDevice();

		DoInit = true;

		//if (line)
			//line->OnResetDevice();

		//stateOnResetDevice(pDevice);
	}

	return Reset__Return;
}

//==========================================================================================================================

HRESULT APIENTRY SetVertexShader_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9 *veShader_)
{
	if (veShader_ != NULL)
	{
		vShader_ = veShader_;
		vShader_->GetFunction(NULL, &vSize_);
	}
	return SetVertexShader_orig(pDevice, veShader_);
}

//==========================================================================================================================

HRESULT APIENTRY SetPixelShader_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9 *piShader_)
{
	if (piShader_ != NULL)
	{
		pShader_ = piShader_;
		pShader_->GetFunction(NULL, &pSize_);
	}
	return SetPixelShader_orig(pDevice, piShader_);
}

//==========================================================================================================================

HRESULT APIENTRY SetTexture_hook(IDirect3DDevice9* pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture_)
{
	//if (pDevice == nullptr) return SetTexture_orig(pDevice, Sampler, pTexture_);

	mStage_ = Sampler;

	if ((es__p > 0 || aim__bot > 0) && (decl_->Type == 8 && num__elements == 3 && mStart__register == 6 && mVector__Count == 2 && Sampler == 0 && pTexture_))//reduce fps loss
	//if((es__p > 0 || aim__bot > 0)&&(decl_->Type == 8 && num__elements == 3 && mStart__register == 6 && mVector__Count == 2 && Sampler == 0 && pTexture_))//reduce fps loss
	{//1
		pCurrentTex = static_cast<IDirect3DTexture9*>(pTexture_);

		if (pCurrentTex)//
		{
			D3DSURFACE_DESC sDesc_;

			//pCurrentTex->GetLevelDesc(0, &sDesc_);
			if (FAILED(pCurrentTex->GetLevelDesc(0, &sDesc_)))
			{
				//Log("sDesc_ failed");
				goto out_;
			}

			if (SUCCEEDED(pCurrentTex->GetLevelDesc(0, &sDesc_)))//
			if (sDesc_.Pool == D3DPOOL_MANAGED && pCurrentTex->GetType() == D3DRTYPE_TEXTURE) //reduce fps loss
			{
				dWidth_ = sDesc_.Width;
				dHeight_ = sDesc_.Height;
				//dFormat_ = sDesc_.Format;

				if (sDesc_.Width == 12 && sDesc_.Height == 8)//reduce fps loss
				{
					D3DLOCKED_RECT pLocked__Rect;
					
					HRESULT hr = pCurrentTex->LockRect(0, &pLocked__Rect, NULL, D3DLOCK_DONOTWAIT | D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
					//if (pCurrentTex->LockRect(0, &pLocked__Rect, NULL, D3DLOCK_DONOTWAIT | D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK) == S_OK)

					if (SUCCEEDED(hr))
					{
						if (pLocked__Rect.pBits != NULL)
						// get crc from the algorithm
						CRC = QuickChecksum((DWORD*)pLocked__Rect.pBits, pLocked__Rect.Pitch);
					}
					pCurrentTex->UnlockRect(0);
				}
			}
			//SAFE_RELEASE(pCurrentTex);
		}
	}
out_:
	//SAFE_RELEASE(pCurrentTex);
	

	return SetTexture_orig(pDevice, Sampler, pTexture_);
}

//==========================================================================================================================

DWORD WINAPI pPD(__in  LPVOID lpParameter)
{
	//HWND pWND_ = NULL;
	//while (!pWND_)
	//{
		//Multi national version compatible 
		//pWND_ = FindWindowA("LaunchUnrealUWindowsClient", 0); //avoid inj i.s.o.g
		//Sleep(100);
	//}
	//CloseHandle(pWND_);

	HMODULE dDll_ = NULL;
	while (!dDll_)
	{
		dDll_ = GetModuleHandleA("d3d9.dll");
		Sleep(100);
	}
	CloseHandle(dDll_);

	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;

	HWND tmpWnd = CreateWindowA("BUTTON", "bD3D", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, Hand, NULL);
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
	DWORD64* dVtable_ = (DWORD64*)d3ddev;
	dVtable_ = (DWORD64*)dVtable_[0];
	#elif defined _M_IX86
	DWORD* dVtable_ = (DWORD*)d3ddev;
	dVtable_ = (DWORD*)dVtable_[0]; // == *d3ddev
	#endif
	//Log("[DirectX] dVtable_: %x", dVtable_);

	//for(int i = 0; i < 95; i++)
	//{
		//Log("[DirectX] vtable[%i]: %x, pointer at %x", i, dVtable_[i], &dVtable_[i]);
	//}

	// Set EndScene_orig to the original EndScene etc.
	SetVertexDeclaration_orig = (SetVertexDeclaration)dVtable_[87];
	EndScene_orig = (EndScene)dVtable_[42];
	SetStreamSource_orig = (SetStreamSource)dVtable_[100];
	SetVertexShaderConstantF_orig = (SetVertexShaderConstantF)dVtable_[94];
	DrawIndexedPrimitive_orig = (DrawIndexedPrimitive)dVtable_[82];
	Reset_orig = (Reset)dVtable_[16];
	CreateQuery_orig = (CreateQuery)dVtable_[118];
	//SetView__port_orig = (SetView__port)dVtable_[47];
	//SetVertexShader_orig = (SetVertexShader)dVtable_[92];
	SetPixelShader_orig = (SetPixelShader)dVtable_[107];
	SetTexture_orig = (SetTexture)dVtable_[65];

	// Detour functions x86 & x64
	if (MH_Initialize() != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[87], &SetVertexDeclaration_hook, reinterpret_cast<void**>(&SetVertexDeclaration_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[87]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[42], &EndScene_hook, reinterpret_cast<void**>(&EndScene_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[42]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[100], &SetStreamSource_hook, reinterpret_cast<void**>(&SetStreamSource_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[100]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[94], &SetVertexShaderConstantF_hook, reinterpret_cast<void**>(&SetVertexShaderConstantF_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[94]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[82], &DrawIndexedPrimitive_hook, reinterpret_cast<void**>(&DrawIndexedPrimitive_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[82]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[16], &Reset_hook, reinterpret_cast<void**>(&Reset_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[16]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[118], &CreateQuery_hook, reinterpret_cast<void**>(&CreateQuery_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[118]) != MH_OK) { return 1; }
	//if (MH_CreateHook((DWORD_PTR*)dVtable_[47], &SetView__port_hook, reinterpret_cast<void**>(&SetView__port_orig)) != MH_OK) { return 1; }
	//if (MH_EnableHook((DWORD_PTR*)dVtable_[47]) != MH_OK) { return 1; }
	//if (MH_CreateHook((DWORD_PTR*)dVtable_[92], &SetVertexShader_hook, reinterpret_cast<void**>(&SetVertexShader_orig)) != MH_OK) { return 1; }
	//if (MH_EnableHook((DWORD_PTR*)dVtable_[92]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[107], &SetPixelShader_hook, reinterpret_cast<void**>(&SetPixelShader_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[107]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable_[65], &SetTexture_hook, reinterpret_cast<void**>(&SetTexture_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable_[65]) != MH_OK) { return 1; }

	//Log("[Detours] Detours attached\n");

	d3ddev->Release();
	d3d->Release();
	DestroyWindow(tmpWnd);
		
	return 1;
}

//==========================================================================================================================

BOOL WINAPI DllMain(HMODULE hinst__DLL, DWORD fdwReason, LPVOID lpvReserved)
{
	//DWORD ModuleBase = (DWORD)hinst__DLL;
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: // A process is loading the DLL.
		Hand = hinst__DLL;
		DisableThreadLibraryCalls(hinst__DLL); // disable unwanted thread notifications to reduce overhead
		GetModuleFileNameA(hinst__DLL, dir, 512);
		for (int i = (int)strlen(dir); i > 0; i--)
		{
			if (dir[i] == '\\')
			{
				dir[i + 1] = 0;
				break;
			}
		}
		//(hinst__DLL);
		CreateThread(0, 0, pPD, 0, 0, 0); //init our hooks
		break;

	case DLL_PROCESS_DETACH: // A process unloads the DLL.
		/*
		if (MH_Uninitialize() != MH_OK) { return 1; }
		if (MH_DisableHook((DWORD_PTR*)dVtable_[42]) != MH_OK) { return 1; }
		*/
		break;
	}
	return TRUE;
}
