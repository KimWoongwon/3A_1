#include <Windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("김웅원_16032020");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

#define VOBJECT_VERSION_1	1
#define VOBJECT_VERSION_2	2

class VObject
{
public:
	virtual void show(HDC hdc, int x, int y)
	{
		TextOut(hdc, x, y, TEXT("이것은 VObject 입니다."), lstrlen(TEXT("이것은 VObject 입니다.")));
	}
};

class VObject_v1 : public VObject
{
public:
	int a, b, c;
	char buffer[1024];
	void show(HDC hdc, int x, int y) 
	{
		TextOut(hdc, x, y, TEXT("이것은 VObject_v1 입니다."), lstrlen(TEXT("이것은 VObject_v1 입니다.")));
	}
};

class VObject_v2 : public VObject
{
public:
	float a, b, c, d;
	char buffer[512];
	void show(HDC hdc, int x, int y)
	{
		TextOut(hdc, x, y, TEXT("이것은 VObject_v2 입니다."), lstrlen(TEXT("이것은 VObject_v2 입니다.")));
	}
};

VObject* CreateObject(int index)
{
	VObject* ptr = nullptr;
	switch (index)
	{
	case VOBJECT_VERSION_1:
		ptr = new VObject_v1();
		break;
	case VOBJECT_VERSION_2:
		ptr = new VObject_v2();
		break;
	}
	return ptr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static PAINTSTRUCT ps;
	
	static VObject_v1* version_01;
	static VObject_v2* version_02;

	switch (iMessage) {
	case WM_CREATE:
		
		version_01 = (VObject_v1*)CreateObject(VOBJECT_VERSION_1);
		version_02 = (VObject_v2*)CreateObject(VOBJECT_VERSION_2);

		
		
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		version_01->show(hdc, 20, 20);
		version_02->show(hdc, 20, 50);

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:	// 가장 중요하다고 생각하는 PAINT 메시지 입니다.
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}