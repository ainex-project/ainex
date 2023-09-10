#include "XeniaUWP.h"

#include "windowed_app_context_uwp.h"
#include "xenia/emulator.h"
#include "xenia/ui/windowed_app.h"
#include "xenia/ui/imgui_drawer.h"
#include "xenia/ui/immediate_drawer.h"
#include "xenia/base/cvar.h"
#include "xenia/ui/window.h"
#include "window_uwp.h"
#include "xenia/ui/d3d12/d3d12_provider.h"
#include "xenia/gpu/d3d12/d3d12_graphics_system.h"
#include "xenia/hid/xinput/xinput_hid.h"
#include "xenia/hid/nop/nop_hid.h"
#include "xenia/apu/xaudio2/xaudio2_audio_system.h"
#include "surface_uwp.h"
#include "xenia/config.h"

using namespace xe;

template <typename T, typename... Args>
class Factory {
 private:
  struct Creator {
    std::string name;
    std::function<bool()> is_available;
    std::function<std::unique_ptr<T>(Args...)> instantiate;
  };

  std::vector<Creator> creators_;

 public:
  void Add(const std::string_view name, std::function<bool()> is_available,
           std::function<std::unique_ptr<T>(Args...)> instantiate) {
    creators_.push_back({std::string(name), is_available, instantiate});
  }

  void Add(const std::string_view name,
           std::function<std::unique_ptr<T>(Args...)> instantiate) {
    auto always_available = []() { return true; };
    Add(name, always_available, instantiate);
  }

  template <typename DT>
  void Add(const std::string_view name) {
    Add(name, DT::IsAvailable, [](Args... args) {
      return std::make_unique<DT>(std::forward<Args>(args)...);
    });
  }

  std::unique_ptr<T> Create(const std::string_view name, Args... args) {
    if (!name.empty() && name != "any") {
      auto it = std::find_if(
          creators_.cbegin(), creators_.cend(),
          [&name](const auto& f) { return name.compare(f.name) == 0; });
      if (it != creators_.cend() && (*it).is_available()) {
        return (*it).instantiate(std::forward<Args>(args)...);
      }
      return nullptr;
    } else {
      for (const auto& creator : creators_) {
        if (!creator.is_available()) continue;
        auto instance = creator.instantiate(std::forward<Args>(args)...);
        if (!instance) continue;
        return instance;
      }
      return nullptr;
    }
  }

  std::vector<std::unique_ptr<T>> CreateAll(const std::string_view name,
                                            Args... args) {
    std::vector<std::unique_ptr<T>> instances;
    if (!name.empty() && name != "any") {
      auto it = std::find_if(
          creators_.cbegin(), creators_.cend(),
          [&name](const auto& f) { return name.compare(f.name) == 0; });
      if (it != creators_.cend() && (*it).is_available()) {
        auto instance = (*it).instantiate(std::forward<Args>(args)...);
        if (instance) {
          instances.emplace_back(std::move(instance));
        }
      }
    } else {
      for (const auto& creator : creators_) {
        if (!creator.is_available()) continue;
        auto instance = creator.instantiate(std::forward<Args>(args)...);
        if (instance) {
          instances.emplace_back(std::move(instance));
        }
      }
    }
    return instances;
  }
};

std::unique_ptr<gpu::GraphicsSystem> CreateGraphicsSystem() {
  Factory<gpu::GraphicsSystem> factory;
#if XE_PLATFORM_WIN32
  factory.Add<gpu::d3d12::D3D12GraphicsSystem>("d3d12");
#endif
  return factory.Create("any");
}

std::vector<std::unique_ptr<hid::InputDriver>> CreateInputDrivers(
    ui::Window* window) {
  std::vector<std::unique_ptr<hid::InputDriver>> drivers;

  Factory<hid::InputDriver, ui::Window*, size_t> factory;

  factory.Add("xinput", xe::hid::xinput::Create);

  for (auto& driver : factory.CreateAll("any", window, 1)) {
    if (XSUCCEEDED(driver->Setup())) {
      drivers.emplace_back(std::move(driver));
    }
  }

  return drivers;
}

std::unique_ptr<apu::AudioSystem> CreateAudioSystem(
    cpu::Processor* processor) {
  Factory<apu::AudioSystem, cpu::Processor*> factory;
#if XE_PLATFORM_WIN32
  factory.Add<apu::xaudio2::XAudio2AudioSystem>("xaudio2");
#endif  // XE_PLATFORM_WIN32
  return factory.Create("any", processor);
}

static std::unique_ptr<Emulator> emu = nullptr;
static std::unique_ptr<ui::WindowedApp> app = nullptr;
static std::unique_ptr<ui::UWPWindowedAppContext> app_context = nullptr;
static std::unique_ptr<ui::Window> window = nullptr;
static std::unique_ptr<ui::ImGuiDrawer> imgui = nullptr;
static std::unique_ptr<ui::ImmediateDrawer> immediate_drawer = nullptr;
void UWP::XeniaUWP::StartXenia(std::string gamePath, std::string configPath, uint32_t width,
                               uint32_t height) {

  app_context = std::make_unique<ui::UWPWindowedAppContext>();
  app = xe::ui::GetWindowedAppCreator()(*app_context.get());

  window = std::move(xe::ui::UWPWindow::Create(*app_context, "", width, height));

  config::SetupConfig(configPath);

  emu = std::make_unique<xe::Emulator>(
      configPath, configPath, configPath, configPath);
  window->SetMainMenuEnabled(true);

  // Needed for later
  //imgui = std::make_unique<ui::ImGuiDrawer>(window.get(), 1);

  emu->Setup(window.get(), nullptr, false, CreateAudioSystem,
             CreateGraphicsSystem,
                  CreateInputDrivers);

  app_context->CallInUIThread([this]() {
    gpu::GraphicsSystem* graphics_system = emu->graphics_system();
    ui::Presenter* presenter = graphics_system->presenter();
    if (!presenter) {
      return;
    }

    //immediate_drawer = graphics_system->provider()->CreateImmediateDrawer();
    //if (immediate_drawer) {
    //  immediate_drawer->SetPresenter(presenter);
    //  imgui->SetPresenterAndImmediateDrawer(presenter, immediate_drawer.get());
    //}

    window->Open();
    window->SetPresenter(presenter);
  });

  emu->LaunchPath(gamePath);
}

void UWP::XeniaUWP::ExecutePendingFunctionsFromUIThread() {
  app_context->ExecutePendingFunctionsFromUIThread();
}
