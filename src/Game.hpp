//
// Created by kshitij on 11/9/25.
//

#pragma once
#include "Camera.hpp"
#include "LevelGenerator.hpp"
#include "NetworkClient.hpp"
#include "World.hpp"

class Player;

class Game {
    public:
    Game(LevelGenerator &lvlGen, World &world, Player &player);

    LevelGenerator& lvlGen;
    World& world;
    Player& player;

    int Loop(NetworkClient *networkClient, CameraManager &camera_manager);
};
