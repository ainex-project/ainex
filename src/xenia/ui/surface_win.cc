/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/ui/surface_win.h"

#include <winrt/windows.graphics.display.core.h>
#include <Gamingdeviceinformation.h>

namespace xe {
namespace ui {

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_GAMES)
bool Win32HwndSurface::GetSizeImpl(uint32_t& width_out,
                                   uint32_t& height_out) const {
  width_out = 1920;
  height_out = 1080;
  GAMING_DEVICE_MODEL_INFORMATION info = {};
  GetGamingDeviceModelInformation(&info);
  if (info.vendorId == GAMING_DEVICE_VENDOR_ID_MICROSOFT) {
    auto hdi = winrt::Windows::Graphics::Display::Core::HdmiDisplayInformation::
        GetForCurrentView();
    if (hdi) {
      width_out = hdi.GetCurrentDisplayMode().ResolutionWidthInRawPixels();
      height_out = hdi.GetCurrentDisplayMode().ResolutionHeightInRawPixels();
    }
  }

  return true;
}
#endif

}  // namespace ui
}  // namespace xe
