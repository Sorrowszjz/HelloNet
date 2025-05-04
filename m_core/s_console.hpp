#ifndef __HELLO_NET_M_CORE_S_CONSOLE__
#define __HELLO_NET_M_CORE_S_CONSOLE__
#include "s_base.hpp"
#include "s_stringutil.hpp"



/**
*@brief  控制台工具
*/
class CHNConsoleUtil : CHNNoCopyable
{
public:

	//将光标移动到坐标为(x,y)的地方
	static bool ConsoleMove(int x, int y)
	{
#if defined(HNOS_WIN)
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
		HANDLE hConsoleOut;
		hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(hConsoleOut, &csbiInfo);
		csbiInfo.dwCursorPosition.X = x;
		csbiInfo.dwCursorPosition.Y = y;
		SetConsoleCursorPosition(hConsoleOut, csbiInfo.dwCursorPosition);
		return true;
#else
		return false;
#endif
	}

	//清理控制台
	static bool ConsoleClear()
	{
#if defined(HNOS_WIN)
		COORD coordScreen = { 0, 0 };    // home for the cursor 
		DWORD cCharsWritten;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		DWORD dwConSize;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		// Get the number of character cells in the current buffer. 
		if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
			return false;
		dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

		// Fill the entire screen with blanks.
		if (!FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
			dwConSize, coordScreen, &cCharsWritten))
			return false;

		// Get the current text attribute.
		if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
			return false;

		// Set the buffer's attributes accordingly.
		if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes,
			dwConSize, coordScreen, &cCharsWritten))
			return false;

		// Put the cursor at its home coordinates.
		SetConsoleCursorPosition(hConsole, coordScreen);		
		return true;
#else
		return false;
#endif
	}
};


/**
*@brief  控制台颜色控制，对象析构后下一行自动还原
*@param  bSucc  [IN]	成功时变为绿色，失败时变为红色
*/
class CHNAutoConsoleColor : CHNNoCopyable
{
public:
	CHNAutoConsoleColor(bool bSucc)
	{
#if defined(HNOS_WIN)
		if (bSucc)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
		else
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
#endif
	}
	~CHNAutoConsoleColor()
	{
#if defined(HNOS_WIN)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	}
};



#endif // __HELLO_NET_M_CORE_S_CONSOLE__