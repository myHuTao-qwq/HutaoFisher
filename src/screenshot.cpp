#include "screenshot.h"
using cv::Mat;

Screen::Screen() {
  // 获取原神窗口的句柄
  HWND hWnd = FindHandleByWindowName();

  // 获取游戏区域
  RECT client_rect = { 0, 0, 0, 0 };
  GetClientRect(hWnd, &client_rect);

  m_width = client_rect.right - client_rect.left;
  m_height = client_rect.bottom - client_rect.top;
  m_screenshotData = new char[m_width * m_height * 4];
  memset(m_screenshotData, 0, m_width);

  // 获取句柄 DC // 建议下沉至 getScreenshot() 并 DeleteDC + ReleaseDC
  m_screenDC = GetDC(hWnd);
  m_compatibleDC = CreateCompatibleDC(m_screenDC);

  // 创建位图
  m_hBitmap = CreateCompatibleBitmap(m_screenDC, m_width, m_height);
  SelectObject(m_compatibleDC, m_hBitmap);
}

/** 
 * 获取原神窗口的句柄
 * 此方法获取的句柄有概率造成原神 10258-4001 错误
 */
HWND FindHandleByWindowName() {
  HWND handle = FindWindowA("UnityWndClass", "原神");
  if (handle != 0) {
      return handle;
  }
  
  handle = FindWindowA("UnityWndClass", "Genshin Impact");
  if (handle != 0) {
      return handle;
  }
  
  handle = FindWindowA("Qt5152QWindowIcon", "云·原神");
  if (handle != 0) {
      return handle;
  }
  
  return NULL;
}

/* 获取整个屏幕的截图 */
Mat Screen::getScreenshot() {
  // 得到位图的数据
  BitBlt(m_compatibleDC, 0, 0, m_width, m_height, m_screenDC, 0, 0, SRCCOPY);
  GetBitmapBits(m_hBitmap, m_width * m_height * 4, m_screenshotData);

  // 创建图像
  Mat origin(m_height, m_width, CV_8UC4, m_screenshotData);
  // Mat screenshot(m_height, m_width, CV_8UC3);
  Mat screenshot;
  cv::cvtColor(origin, screenshot, cv::COLOR_BGRA2BGR);

  return screenshot;
}

/** @brief 获取指定范围的屏幕截图
 * @param x 图像左上角的 X 坐标
 * @param y 图像左上角的 Y 坐标
 * @param width 图像宽度
 * @param height 图像高度
 */
Mat Screen::getScreenshot(int x, int y, int width, int height) {
  Mat screenshot = getScreenshot();
  return screenshot(cv::Rect(x, y, width, height));
}