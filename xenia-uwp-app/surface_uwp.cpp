#include "surface_uwp.h"

xe::ui::UWPWindowSurface::UWPWindowSurface() { }

bool xe::ui::UWPWindowSurface::GetSizeImpl(uint32_t& width_out,
                                           uint32_t& height_out) const {
  width_out = 1920;
  height_out = 1080;
  return true;
}