//
// Created by kshitij on 11/9/25.
//

#include "Lobby.hpp"

#include <imgui.h>
#include <iostream>
#include <bits/ostream.tcc>

#include "Camera.hpp"
#include "Global.hpp"
#include "NetworkClient.hpp"
#include "rlImGui.h"

Lobby::Lobby(LevelGenerator& lvlGen, World& world, Player& player): world(world), player(player) {
    lvlGen.GenerateFromImage(ASSET_DIR "/levels/lobby.png", &world);
    lvlGen.SetPlayerPosition(player);
}

bool Lobby::IsEveryoneReady() {
    return false;
}

Vector2 Lerp(Vector2 a, Vector2 b, float t) {
    return a + (b - a) * t;
}

void Lobby::UpdateOtherPlayers() {
    for (const auto& [name, settings] : Global::Get().players) {
        if (name == player.username) continue;

        if (!otherPlayers.contains(name)) {
            otherPlayers[name] = Player(settings.position);
            otherPlayers[name].username = name;
        }

        otherPlayers[name].position = Lerp(otherPlayers[name].position, settings.position, 0.6f);
        // std::cout << name << " updated pos: " << settings.position.x << ", " << settings.position.y << std::endl;
    }
    // std::cout << std::endl;
}

bool ready;
void Lobby::Loop(NetworkClient& networkClient, CameraManager& camera_manager) {
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();
        // Update
        //----------------------------------------------------------------------------------
        player.GetInputs();

        world.Update(dt);
        player.Update(dt);

        player.CheckCollisions(world);
        networkClient.SendPacket("3|" + player.username + "|[" + std::to_string(player.position.x) + "," + std::to_string(player.position.y) + "]:");

        camera_manager.Update(player.position);
        UpdateOtherPlayers();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera_manager.camera);
        world.Draw();
        Vector2 mousePosition = GetScreenToWorld2D(GetMousePosition(), camera_manager.camera);
        player.Draw(&mousePosition);

        for (const auto& [name, settings] : Global::Get().players) {
            if (name == player.username) continue;
            if (otherPlayers.contains(name)) otherPlayers[name].Draw();
        }

        EndMode2D();

        // Debug
        // -----------------------------------------------------------------------------
        DrawFPS(10, 10);
        // -----------------------------------------------------------------------------

        // start ImGui HUD
        rlImGuiBegin();

        ImGui::Begin("Settings");
        ImGui::Checkbox("Ready", &ready);

        // Only send once
        if (!isReady && ready) {
            isReady = true;
            std::cout << "Player " << player.username << "is ready." << std::endl;
            networkClient.SendPacket("2|" + player.username + "|1");
        }
        if (isReady && !ready) {
            isReady = false;
            std::cout << "Player " << player.username << "is not ready." << std::endl;
            networkClient.SendPacket("2|" + player.username + "|0");
        }

        ImGui::End();

        // end ImGui Content
        rlImGuiEnd();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    networkClient.SendPacket("4|" + player.username);
    CloseWindow();        // Close window and OpenGL context
}