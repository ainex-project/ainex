/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2018 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_GPU_D3D12_RENDER_TARGET_CACHE_H_
#define XENIA_GPU_D3D12_RENDER_TARGET_CACHE_H_

#include <unordered_map>

#include "xenia/gpu/d3d12/shared_memory.h"
#include "xenia/gpu/register_file.h"
#include "xenia/gpu/xenos.h"
#include "xenia/ui/d3d12/d3d12_api.h"

namespace xe {
namespace gpu {
namespace d3d12 {

class D3D12CommandProcessor;

// =============================================================================
// How EDRAM is used by Xenos:
// (Copied from the old version of the render target cache, so implementation
//  info may differ from the way EDRAM is emulated now.)
// =============================================================================
//
// On the 360 the render target is an opaque block of memory in EDRAM that's
// only accessible via resolves. We use this to our advantage to simulate
// something like it as best we can by having a shared backing memory with
// a multitude of views for each tile location in EDRAM.
//
// This allows us to have the same base address write to the same memory
// regardless of framebuffer format. Resolving then uses whatever format the
// resolve requests straight from the backing memory.
//
// EDRAM is a beast and we only approximate it as best we can. Basically,
// the 10MiB of EDRAM is composed of 2048 5120b tiles. Each tile is 80x16px.
// +-----+-----+-----+---
// |tile0|tile1|tile2|...  2048 times
// +-----+-----+-----+---
// Operations dealing with EDRAM deal in tile offsets, so base 0x100 is tile
// offset 256, 256*5120=1310720b into the buffer. All rendering operations are
// aligned to tiles so trying to draw at 256px wide will have a real width of
// 320px by rounding up to the next tile.
//
// MSAA and other settings will modify the exact pixel sizes, like 4X makes
// each tile effectively 40x8px / 2X makes each tile 80x8px, but they are still
// all 5120b. As we try to emulate this we adjust our viewport when rendering to
// stretch pixels as needed.
//
// It appears that games also take advantage of MSAA stretching tiles when doing
// clears. Games will clear a view with 1/2X pitch/height and 4X MSAA and then
// later draw to that view with 1X pitch/height and 1X MSAA.
//
// The good news is that games cannot read EDRAM directly but must use a copy
// operation to get the data out. That gives us a chance to do whatever we
// need to (re-tile, etc) only when requested.
//
// To approximate the tiled EDRAM layout we use a single large chunk of memory.
// From this memory we create many VkImages (and VkImageViews) of various
// formats and dimensions as requested by the game. These are used as
// attachments during rendering and as sources during copies. They are also
// heavily aliased - lots of images will reference the same locations in the
// underlying EDRAM buffer. The only requirement is that there are no hazards
// with specific tiles (reading/writing the same tile through different images)
// and otherwise it should be ok *fingers crossed*.
//
// One complication is the copy/resolve process itself: we need to give back
// the data asked for in the format desired and where it goes is arbitrary
// (any address in physical memory). If the game is good we get resolves of
// EDRAM into fixed base addresses with scissored regions. If the game is bad
// we are broken.
//
// Resolves from EDRAM result in tiled textures - that's texture tiles, not
// EDRAM tiles. If we wanted to ensure byte-for-byte correctness we'd need to
// then tile the images as we wrote them out. For now, we just attempt to
// get the (X, Y) in linear space and do that. This really comes into play
// when multiple resolves write to the same texture or memory aliased by
// multiple textures - which is common due to predicated tiling. The examples
// below demonstrate what this looks like, but the important thing is that
// we are aware of partial textures and overlapping regions.
//
// Example with multiple render targets:
//   Two color targets of 256x256px tightly packed in EDRAM:
//     color target 0: base 0x0, pitch 320, scissor 0,0, 256x256
//       starts at tile 0, buffer offset 0
//       contains 64 tiles (320/80)*(256/16)
//     color target 1: base 0x40, pitch 320, scissor 256,0, 256x256
//       starts at tile 64 (after color target 0), buffer offset 327680b
//       contains 64 tiles
//   In EDRAM each set of 64 tiles is contiguous:
//     +------+------+   +------+------+------+
//     |ct0.0 |ct0.1 |...|ct0.63|ct1.0 |ct1.1 |...
//     +------+------+   +------+------+------+
//   To render into these, we setup two VkImages:
//     image 0: bound to buffer offset 0, 320x256x4=327680b
//     image 1: bound to buffer offset 327680b, 320x256x4=327680b
//   So when we render to them:
//     +------+-+ scissored to 256x256, actually 320x256
//     | .    | | <- . appears at some untiled offset in the buffer, but
//     |      | |      consistent if aliased with the same format
//     +------+-+
//   In theory, this gives us proper aliasing in most cases.
//
// Example with horizontal predicated tiling:
//   Trying to render 1024x576 @4X MSAA, splitting into two regions
//   horizontally:
//     +----------+
//     | 1024x288 |
//     +----------+
//     | 1024x288 |
//     +----------+
//   EDRAM configured for 1056x288px with tile size 2112x567px (4X MSAA):
//     color target 0: base 0x0, pitch 1080, 26x36 tiles
//   First render (top):
//     window offset 0,0
//     scissor 0,0, 1024x288
//   First resolve (top):
//     RB_COPY_DEST_BASE    0x1F45D000
//     RB_COPY_DEST_PITCH   pitch=1024, height=576
//     vertices: 0,0, 1024,0, 1024,288
//   Second render (bottom):
//     window offset 0,-288
//     scissor 0,288, 1024x288
//   Second resolve (bottom):
//     RB_COPY_DEST_BASE    0x1F57D000 (+1179648b)
//     RB_COPY_DEST_PITCH   pitch=1024, height=576
//       (exactly 1024x288*4b after first resolve)
//     vertices: 0,288, 1024,288, 1024,576
//   Resolving here is easy as the textures are contiguous in memory. We can
//   snoop in the first resolve with the dest height to know the total size,
//   and in the second resolve see that it overlaps and place it in the
//   existing target.
//
// Example with vertical predicated tiling:
//   Trying to render 1280x720 @2X MSAA, splitting into two regions
//   vertically:
//     +-----+-----+
//     | 640 | 640 |
//     |  x  |  x  |
//     | 720 | 720 |
//     +-----+-----+
//   EDRAM configured for 640x736px with tile size 640x1472px (2X MSAA):
//     color target 0: base 0x0, pitch 640, 8x92 tiles
//   First render (left):
//     window offset 0,0
//     scissor 0,0, 640x720
//   First resolve (left):
//     RB_COPY_DEST_BASE    0x1BC6D000
//     RB_COPY_DEST_PITCH   pitch=1280, height=720
//     vertices: 0,0, 640,0, 640,720
//   Second render (right):
//     window offset -640,0
//     scissor 640,0, 640x720
//   Second resolve (right):
//     RB_COPY_DEST_BASE    0x1BC81000 (+81920b)
//     RB_COPY_DEST_PITCH   pitch=1280, height=720
//     vertices: 640,0, 1280,0, 1280,720
//   Resolving here is much more difficult as resolves are tiled and the right
//   half of the texture is 81920b away:
//     81920/4bpp=20480px, /32 (texture tile size)=640px
//   We know the texture size with the first resolve and with the second we
//   must check for overlap then compute the offset (in both X and Y).
//
// =============================================================================
// Surface size:
// =============================================================================
//
// XGSurfaceSize code in game executables calculates the size in tiles in the
// following order:
// 1) If MSAA is >=2x, multiply the height by 2.
// 2) If MSAA is 4x, multiply the width by 2.
// 3) 80x16-align multisampled width and height.
// 4) Multiply width*height by 4 or 8 depending on the pixel format.
// 5) Divide the byte size by 5120.
// This means that when working with EDRAM surface sizes we should assume that a
// multisampled surface is the same as a single-sampled surface with 2x height
// and width - however, format size doesn't effect the dimensions. Surface pitch
// in the surface info register is single-sampled.
class RenderTargetCache {
 public:
  RenderTargetCache(D3D12CommandProcessor* command_processor,
                    RegisterFile* register_file);
  ~RenderTargetCache();

  bool Initialize();
  void Shutdown();
  void ClearCache();

  void BeginFrame();
  // Called in the beginning of a draw call - may bind pipelines.
  void UpdateRenderTargets();
  void EndFrame();

  static inline bool IsColorFormat64bpp(ColorRenderTargetFormat format) {
    return format == ColorRenderTargetFormat::k_16_16_16_16 ||
           format == ColorRenderTargetFormat::k_16_16_16_16_FLOAT ||
           format == ColorRenderTargetFormat::k_32_32_FLOAT;
  }
  static DXGI_FORMAT GetColorDXGIFormat(ColorRenderTargetFormat format);

 private:
  union RenderTargetKey {
    struct {
      // Supersampled dimensions. The limit is 2560x2560 without AA, 2560x5120
      // with 2x AA, and 5120x5120 with 4x AA.
      uint32_t width_ss_div_80 : 7;   // 7
      uint32_t height_ss_div_16 : 9;  // 16
      uint32_t is_depth : 1;          // 17
      uint32_t format : 4;            // 21
    };
    uint32_t value;

    // Clearing the unused bits.
    RenderTargetKey() : value(0) {}
    RenderTargetKey(const RenderTargetKey& key) : value(key.value) {}
    RenderTargetKey& operator=(const RenderTargetKey& key) {
      value = key.value;
      return *this;
    }
    bool operator==(const RenderTargetKey& key) const {
      return value == key.value;
    }
    bool operator!=(const RenderTargetKey& key) const {
      return value != key.value;
    }
  };

  struct RenderTarget {
    ID3D12Resource* resource;
    D3D12_RESOURCE_STATES state;
    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    RenderTargetKey key;
    // The first 4 MB page in the heaps.
    uint32_t heap_page_first;
    // Number of 4 MB pages this render target uses.
    uint32_t heap_page_count;
  };

  struct RenderTargetBinding {
    // Whether this render target has been used since the last full update.
    bool is_bound;
    uint32_t edram_base;
    // How many tiles has already been drawn to the render target since the last
    // full update.
    uint32_t edram_dirty_length;
    union {
      uint32_t format;
      ColorRenderTargetFormat color_format;
      DepthRenderTargetFormat depth_format;
    };
    RenderTarget* render_target;
  };

  void ClearBindings();

  // Must be in a frame to call. Writes the dirty areas of the currently bound
  // render targets and marks them as clean.
  void WriteRenderTargetsToEDRAM();

  D3D12CommandProcessor* command_processor_;
  RegisterFile* register_file_;

  // 32 MB heaps backing used render targets resources, created when needed.
  // 24 MB proved to be not enough to store a single render target occupying the
  // entire EDRAM - a 32-bit depth/stencil one - at some resolution.
  ID3D12Heap* heaps_[5] = {};

  std::unordered_multimap<uint32_t, RenderTarget*> render_targets_;

  uint32_t current_surface_pitch_ = 0;
  MsaaSamples current_msaa_samples_ = MsaaSamples::k1X;
  RenderTargetBinding current_bindings_[5] = {};
};

}  // namespace d3d12
}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_D3D12_RENDER_TARGET_CACHE_H_