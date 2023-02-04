/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <memory>
#include <string>

#include "xenia/ui/d3d12/d3d12_provider.h"
#include "xenia/ui/window_demo.h"

namespace xe {
namespace ui {
namespace d3d12 {

class UWPWindowApp final : public WindowedApp {
 public:
  static std::unique_ptr<WindowedApp> Create(WindowedAppContext& app_context) {
    return std::unique_ptr<WindowedApp>(new UWPWindowApp(app_context));
  }

 protected:
  std::unique_ptr<GraphicsProvider> CreateGraphicsProvider() const;

 private:
  UWPWindowApp(WindowedAppContext& app_context)
      : WindowedApp(app_context, "xenia-ui-window-uwp") {}

  ~UWPWindowApp() {}

  bool OnInitialize() {
    return true;
  }

  class WindowDemoDialog final : public ImGuiDialog {
   public:
    explicit WindowDemoDialog(ImGuiDrawer* imgui_drawer)
        : ImGuiDialog(imgui_drawer) {}

   protected:
    void OnDraw(ImGuiIO& io) {

    }

    void OnClosing(UIEvent& e) {
    }

    void OnKeyDown(KeyEvent& e) {

    }
  };
};

std::unique_ptr<GraphicsProvider> UWPWindowApp::CreateGraphicsProvider()
    const {
  return D3D12Provider::Create();
}

}  // namespace d3d12
}  // namespace ui
}  // namespace xe

XE_DEFINE_WINDOWED_APP(xenia_ui_window_uwp,
                       xe::ui::d3d12::UWPWindowApp::Create);
