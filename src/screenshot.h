#pragma once
#include <Windows.h>
#include <TlHelp32.h>

#include <opencv2/opencv.hpp>

class Screen {
 public:
  Screen();
  void init();
  bool working;
  HWND FindHandle(const std::string& processName);
  HWND GetHwndByPid(DWORD dwProcessID);
  cv::Mat getScreenshot();
  cv::Mat getScreenshot(int x, int y, int width, int height);
  int m_width;
  int m_height;
  HWND gameHandle;

 private:
  HDC m_screenDC;
  HDC m_compatibleDC;
  HBITMAP m_hBitmap;
  LPVOID m_screenshotData = nullptr;
};

class screenshotException : public std::exception {
 public:
  using std::exception::exception;
};