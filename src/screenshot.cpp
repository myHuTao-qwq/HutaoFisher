#include "screenshot.h"
#include "shellscalingapi.h"

using cv::Mat;

Screen::Screen() {
  // 获取原神窗口的句柄
  HWND hWnd = FindHandle("YuanShen.exe");
  this->gameHandle = hWnd;
  // dpi 感知
  SetProcessDpiAwareness(PROCESS_DPI_UNAWARE);
}

/** 
 * 获取原神窗口的句柄
 */
HWND Screen::FindHandle(const std::string& processName) {
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
      std::cerr << "CreateToolhelp32Snapshot failed." << std::endl;
      return NULL;
  }

  pe32.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(hProcessSnap, &pe32)) {
      CloseHandle(hProcessSnap);
      std::cerr << "Process32First failed." << std::endl;
      return NULL;
  }

  do {
      if (processName == pe32.szExeFile) {
          return GetHwndByPid(pe32.th32ProcessID);
      }
  } while (Process32Next(hProcessSnap, &pe32));

  CloseHandle(hProcessSnap);
  return NULL;
}

// 根据进程ID获取窗口句柄
HWND Screen::GetHwndByPid(DWORD processID) {
    HWND hwnd = NULL;
    hwnd = FindWindow(NULL, NULL);
    while (hwnd) {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == processID) {
            // 检查是否是主窗体
            HWND parent = GetAncestor(hwnd, GA_ROOTOWNER);
            if (parent == hwnd || parent == NULL) {
                return hwnd;
            }
        }
        hwnd = FindWindowEx(NULL, hwnd, NULL, NULL);
    }
    return NULL;
}

/* 获取整个屏幕的截图 */
Mat Screen::getScreenshot() {
  if (this->gameHandle == NULL) {
	  return Mat();
  }
  // 获取游戏区域
  RECT client_rect;
  GetClientRect(this->gameHandle, &client_rect);

  m_width = client_rect.right - client_rect.left;
  m_height = client_rect.bottom - client_rect.top;

  if (m_width == 0 || m_height == 0) {
      return Mat();
  }

  m_screenshotData = new char[m_width * m_height * 4];
  memset(m_screenshotData, 0, m_width);
 
  // 获取句柄 DC // 建议下沉至 getScreenshot() 并 DeleteDC + ReleaseDC
  m_screenDC = GetDC(this->gameHandle);
  m_compatibleDC = CreateCompatibleDC(m_screenDC);
 
  // 创建位图
  m_hBitmap = CreateCompatibleBitmap(m_screenDC, m_width, m_height);
  SelectObject(m_compatibleDC, m_hBitmap);
  // 得到位图的数据
  BitBlt(m_compatibleDC, 0, 0, m_width, m_height, m_screenDC, 0, 0, SRCCOPY);
  GetBitmapBits(m_hBitmap, m_width * m_height * 4, m_screenshotData);

  // 创建图像
  Mat origin(m_height, m_width, CV_8UC4, m_screenshotData);
  // Mat screenshot(m_height, m_width, CV_8UC3);
  Mat screenshot;
  cv::cvtColor(origin, screenshot, cv::COLOR_BGRA2BGR);

  // 释放资源
  DeleteObject(m_hBitmap);
  DeleteDC(m_compatibleDC);
  ReleaseDC(this->gameHandle, m_screenDC);
  delete[] m_screenshotData;

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