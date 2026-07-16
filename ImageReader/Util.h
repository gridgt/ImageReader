#pragma once
class Util
{
public:
	static bool isWin11();
	static HWND getWorkerW();
	static RAWINPUT* getRawInput(HRAWINPUT lParam);
	static void regInputDevice(HWND hwnd);
};

