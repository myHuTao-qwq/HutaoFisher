#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <dwmapi.h>

#include <iostream>
#include <opencv2/opencv.hpp>

#include "shellscalingapi.h"

class Screen {
 public:
  Screen();
  void init();
  bool working;
  cv::Mat getScreenshot();
  cv::Mat getScreenshot(int x, int y, int width, int height);
  int m_width;
  int m_height;
  std::pair<double, double> getAbsPos(double x, double y);

 private:
  HDC m_screenDC;
  HDC m_compatibleDC;
  HBITMAP m_hBitmap;
  LPVOID m_screenshotData = nullptr;
  HWND gameHandle;
  HWND FindHandle(const std::string& processName);
  HWND GetHwndByPid(DWORD dwProcessID);
  RECT GetCaptureRect(HWND hWnd);
};

class screenshotException : public std::exception {
 public:
  using std::exception::exception;
};