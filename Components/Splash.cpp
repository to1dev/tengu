#include "Splash.h"

namespace Daitengu::Components {

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)

HBITMAP QImageToHBITMAP(const QImage& image)
{
    QImage convertedImage = image.convertToFormat(QImage::Format_RGBA8888);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = convertedImage.width();
    bmi.bmiHeader.biHeight = -convertedImage.height();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(NULL);
    HBITMAP hBitmap = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT,
        convertedImage.constBits(), &bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    return hBitmap;
}

HBITMAP QImageToHBITMAPWithPremultipliedAlpha(const QImage& image)
{
    QImage premultipliedImage
        = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = premultipliedImage.width();
    bmi.bmiHeader.biHeight = -premultipliedImage.height();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(NULL);
    HBITMAP hBitmap = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT,
        premultipliedImage.constBits(), &bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    return hBitmap;
}
#endif

Splash::Splash(const QPixmap& pixmap)
    : QSplashScreen(pixmap)
{
    HDC dc;
    RECT rc;
    POINT pt;
    POINT pt2;
    SIZE sz;
    HWND handle = (HWND)winId();
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
    SetWindowLong(handle, GWL_EXSTYLE,
        GetWindowLong(handle, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetWindowPos(handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    dc = GetDC(0);
    GetWindowRect(handle, &rc);
    pt.x = rc.left;
    pt.y = rc.top;
    pt2.x = 0;
    pt2.y = 0;
    sz.cx = pixmap.width();
    sz.cy = pixmap.height();

    HDC dc2 = CreateCompatibleDC(dc);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    SelectObject(
        dc2, QtWin::toHBITMAP(pixmap, QtWin::HBitmapPremultipliedAlpha));
#else
    SelectObject(dc2, QImageToHBITMAPWithPremultipliedAlpha(pixmap.toImage()));
#endif
    UpdateLayeredWindow(handle, dc, &pt, &sz, dc2, &pt2, 0, &bf, ULW_ALPHA);
    ReleaseDC(0, dc);
    ReleaseDC(0, dc2);
}

void Splash::stayOnTop()
{
    SetWindowPos(
        (HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

}
