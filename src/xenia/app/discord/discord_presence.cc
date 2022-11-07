/**
******************************************************************************
* Ainex : Xbox 360 Emulator                                                  *
******************************************************************************
* Copyright 2022 Ainex Project. All rights reserved.                         *
* Released under the BSD license - see LICENSE in the root for more details. *
******************************************************************************
*/

#include "discord_presence.h"
#include "third_party/discord-rpc/include/discord_rpc.h"
#include "xenia/base/string.h"

// TODO(halotroop2288): Remove Discord rich presence in favor of Revolt rich presence.
// There is not yet a C++ API for Revolt. Nor rich presence.
namespace xe {
namespace discord {

void HandleDiscordReady(const DiscordUser* request) {}
void HandleDiscordError(int errorCode, const char* message) {}
void HandleDiscordJoinGame(const char* joinSecret) {}
void HandleDiscordJoinRequest(const DiscordUser* request) {}
void HandleDiscordSpectateGame(const char* spectateSecret) {}

void DiscordPresence::Initialize() {
  DiscordEventHandlers handlers = {};
  handlers.ready = &HandleDiscordReady;
  handlers.errored = &HandleDiscordError;
  handlers.joinGame = &HandleDiscordJoinGame;
  handlers.joinRequest = &HandleDiscordJoinRequest;
  handlers.spectateGame = &HandleDiscordSpectateGame;
  // Discord_Initialize("606840046649081857", &handlers, 0, ""); // Ainex does not have an official Discord app ID.
}

void DiscordPresence::NotPlaying() {
  DiscordRichPresence discordPresence = {};
  discordPresence.state = "Idle";
  discordPresence.details = "Standby";
  discordPresence.largeImageKey = "app";
  discordPresence.instance = 1;
  Discord_UpdatePresence(&discordPresence);
}

void DiscordPresence::PlayingTitle(const std::string_view game_title) {
  auto details = std::string(game_title);
  DiscordRichPresence discordPresence = {};
  discordPresence.state = "In Game";
  discordPresence.details = details.c_str();
  // TODO(gibbed): we don't have state icons yet.
  // discordPresence.smallImageKey = "app";
  // discordPresence.largeImageKey = "state_ingame";
  discordPresence.largeImageKey = "app";
  discordPresence.instance = 1;
  Discord_UpdatePresence(&discordPresence);
}

void DiscordPresence::Shutdown() { Discord_Shutdown(); }

}  // namespace discord
}  // namespace xe
