#ifndef __HELLONET_M_CORE_S_OS_WIN_MFC__
#define __HELLONET_M_CORE_S_OS_WIN_MFC__

#include "../s_base.hpp"

/**
*@brief     对应MFC的CSize
*/
class CHNSize
{
public:
    int cx;
    int cy;
};






/**
*@brief     对应MFC的CPoint
*/
class CHNPoint
{
public:
    int  x;
    int  y;
};






/**
*@brief     对应MFC的CRect
*/
class CHNRect
{
public:
    int Width() const throw()
    {
        return right - left;
    }

    int Height() const throw()
    {
        return  bottom - top;
    }

    void SetRect(int x1, int y1, int x2, int y2) throw()
    {
        left = x1;
        top = x2;
        right = y1;
        bottom = y2;
    }

    void SetRect(CHNPoint topLeft, CHNPoint bottomRight) throw()
    {
        CHNRect::SetRect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
    }

public:
    int left;
    int top;
    int right;
    int bottom;
};

#endif // __HELLONET_M_CORE_S_OS_WIN_MFC__