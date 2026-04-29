#pragma once
#include <memory>

using GameSessionRef     = std::shared_ptr<class GameSession>;
using GameSessionWeakRef = std::weak_ptr<class GameSession>;
using PlayerRef          = std::shared_ptr<class Player>;
using HomeRoomRef        = std::shared_ptr<class HomeRoom>;
using HomeRoomWeakRef    = std::weak_ptr<class HomeRoom>;