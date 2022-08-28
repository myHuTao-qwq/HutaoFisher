#pragma once
#include <Windows.h>
#include <opencv2/opencv.hpp>

class Screen
{
public:
    Screen();
    cv::Mat getScreenshot();
    cv::Mat getScreenshot(int x, int y, int width, int height);
    int m_width;
    int m_height;

private:
    
    HDC m_screenDC;
    HDC m_compatibleDC;
    HBITMAP m_hBitmap;
    LPVOID m_screenshotData = nullptr;
};

