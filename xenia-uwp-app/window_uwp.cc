#include "window_uwp.h"
#include "surface_uwp.h"

namespace xe {
namespace ui {
UWPWindow::UWPWindow(WindowedAppContext& app_context,
                     const std::string_view title, uint32_t width,
                     uint32_t height)
    : Window(app_context, title, width, height) {}

UWPWindow::~UWPWindow() {}

std::unique_ptr<Window> UWPWindow::Create(WindowedAppContext& app_context,
                                          const std::string_view title,
                                          uint32_t desired_logical_width,
                                          uint32_t desired_logical_height) {
  return std::make_unique<UWPWindow>(app_context, title, desired_logical_width,
                                     desired_logical_height);
}

uint32_t UWPWindow::GetLatestDpiImpl() const { return 96; }

std::unique_ptr<Surface> UWPWindow::CreateSurfaceImpl(
    Surface::TypeFlags allowed_types) {
  return std::make_unique<UWPWindowSurface>();
}

bool UWPWindow::OpenImpl() { return true; }

void UWPWindow::RequestPaintImpl() {}
void UWPWindow::RequestCloseImpl() {}

void UWPWindow::FocusImpl() {}

}  // namespace ui
}  // namespace xe