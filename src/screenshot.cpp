#include "screenshot.h"
using cv::Mat;

Screen::Screen() {
  // 获取窗口当前显示的监视器
  HWND hWnd = GetDesktopWindow();
  HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

  // 获取监视器逻辑宽度
  MONITORINFOEX monitorInfo;
  monitorInfo.cbSize = sizeof(monitorInfo);
  GetMonitorInfo(hMonitor, &monitorInfo);

  // 获取监视器物理宽度
  DEVMODE dm;
  dm.dmSize = sizeof(dm);
  dm.dmDriverExtra = 0;
  EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &dm);
  int cxPhysical = dm.dmPelsWidth;

  m_width = dm.dmPelsWidth;
  m_height = dm.dmPelsHeight;
  m_screenshotData = new char[m_width * m_height * 4];
  memset(m_screenshotData, 0, m_width);

  // 获取屏幕 DC
  m_screenDC = GetDC(NULL);
  m_compatibleDC = CreateCompatibleDC(m_screenDC);

  // 创建位图
  m_hBitmap = CreateCompatibleBitmap(m_screenDC, m_width, m_height);
  SelectObject(m_compatibleDC, m_hBitmap);
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