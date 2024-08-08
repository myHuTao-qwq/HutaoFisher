#include "screenshot.h"

using cv::Mat;

inline RECT GetWindowRect(HWND hWnd) {
  RECT windowRect;
  DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &windowRect,
                        sizeof(RECT));
  return windowRect;
}

inline RECT GetGameScreenRect(HWND hWnd) {
  RECT clientRect;
  GetClientRect(hWnd, &clientRect);
  return clientRect;
}

Screen::Screen() { this->init(); }

void Screen::init() {
  working = true;
  // 获取原神窗口的句柄
  HWND hWnd = FindHandle("YuanShen.exe");
  this->gameHandle = hWnd;
  // dpi 感知
  SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
}

/**
 * 获取原神窗口的句柄
 */
HWND Screen::FindHandle(const std::string& processName) {
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    std::cerr << "Screenshot warning: CreateToolhelp32Snapshot failed."
              << std::endl;
    return NULL;
  }

  pe32.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(hProcessSnap, &pe32)) {
    CloseHandle(hProcessSnap);
    std::cerr << "Screenshot warning: Process32First failed." << std::endl;
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
    working = false;
    throw screenshotException("the game handle is NULL!");
  }
  // 获取游戏区域
  RECT client_rect;
  GetClientRect(this->gameHandle, &client_rect);

  m_width = client_rect.right - client_rect.left;
  m_height = client_rect.bottom - client_rect.top;

  if (m_width == 0 || m_height == 0) {
    working = false;
    throw screenshotException("the screenshot is empty!");
  }

  if (m_width / 16 != m_height / 9) {
    working = false;
    throw screenshotException(
        "the aspect ratio of the game window is not 16:9!");
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

RECT Screen::GetCaptureRect(HWND hWnd) {
  RECT windowRect = GetWindowRect(hWnd);
  RECT gameScreenRect = GetGameScreenRect(hWnd);
  long left = windowRect.left;
  long top = windowRect.top + (windowRect.bottom - windowRect.top) -
             (gameScreenRect.bottom - gameScreenRect.top);
  long right = left + gameScreenRect.right - gameScreenRect.left;
  long bottom = top + gameScreenRect.bottom - gameScreenRect.top;
  return {left, top, right, bottom};
}

std::pair<double, double> Screen::getAbsPos(double x, double y) {
  int screenWidth, screenHeight;

  HDC hdc = GetDC(NULL);
  screenWidth = GetDeviceCaps(hdc, HORZRES);
  screenHeight = GetDeviceCaps(hdc, VERTRES);
  ReleaseDC(NULL, hdc);

  // 获取游戏捕获区域
  RECT captureArea = GetCaptureRect(this->gameHandle);
  double gameWidth = captureArea.right - captureArea.left;
  double gameHeight = captureArea.bottom - captureArea.top;

  x = (x * 1.0 / 65535 * gameWidth + captureArea.left) * 65535 / screenWidth;
  y = (y * 1.0 / 65535 * gameHeight + captureArea.top) * 65535 / screenHeight;

  return std::pair<double, double>(x, y);
}