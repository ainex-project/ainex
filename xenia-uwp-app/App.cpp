#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <fstream> 

//#include "window_uwp.h"

#include <string>

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/windows.graphics.display.core.h>


#include <Gamingdeviceinformation.h>

#include <XeniaUWP.h>

    //#include "xenia/base/main_win.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Pickers;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    CompositionTarget m_target{ nullptr };
    VisualCollection m_visuals{ nullptr };
    Visual m_selected{ nullptr };
    float2 m_offset{};
    UWP::XeniaUWP m_xenia;
    bool m_xenia_ready = false;

    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const &)
    {
    }

    void Load(hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    winrt::fire_and_forget InitializeXenia() {
      FileOpenPicker openPicker;
      openPicker.ViewMode(PickerViewMode::List);
      openPicker.SuggestedStartLocation(PickerLocationId::ComputerFolder);
      openPicker.FileTypeFilter().Append(L"*");

      auto file = co_await openPicker.PickSingleFileAsync();
      if (file) {
        uint32_t width = 1920;
        uint32_t height = 1080;
        GAMING_DEVICE_MODEL_INFORMATION info = {};
        GetGamingDeviceModelInformation(&info);
        if (info.vendorId == GAMING_DEVICE_VENDOR_ID_MICROSOFT) {
          auto hdi = winrt::Windows::Graphics::Display::Core::
              HdmiDisplayInformation::GetForCurrentView();
          if (hdi) {
            width = hdi.GetCurrentDisplayMode().ResolutionWidthInRawPixels();
            height = hdi.GetCurrentDisplayMode().ResolutionHeightInRawPixels();
          }
        }

        std::string path = winrt::to_string(file.Path().data());

        m_xenia.StartXenia(
            path,
                winrt::to_string(ApplicationData::Current().LocalFolder().Path()),
                width, height);
        m_xenia_ready = true;
      }
    }

    void Run()
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();

        auto navigation = winrt::Windows::UI::Core::SystemNavigationManager::GetForCurrentView();

        // UWP on Xbox One triggers a back request whenever the B button is
        // pressed which can result in the app being suspended if unhandled
        navigation.BackRequested(
            [](const winrt::Windows::Foundation::IInspectable&,
               const BackRequestedEventArgs& args) { args.Handled(true); });
        
        InitializeXenia();

        while (true) {
          window.Dispatcher().ProcessEvents(
              CoreProcessEventsOption::ProcessOneAndAllPending);

          if (m_xenia_ready)
            m_xenia.ExecutePendingFunctionsFromUIThread();

          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void SetWindow(CoreWindow const & window)
    {

    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
