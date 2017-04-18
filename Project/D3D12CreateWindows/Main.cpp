#pragma once
#include "stdafx.h"
#include "CoreFrameWork.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	CoreFrameWork sample(1280, 720, L"D3D12 Hello Triangle");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}