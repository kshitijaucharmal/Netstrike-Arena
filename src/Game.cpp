//
// Created by kshitij on 11/9/25.
//

#include "Game.hpp"

#include "Global.hpp"
#include "imgui.h"
#include "rlImGui.h"

Game::Game(LevelGenerator &lvlGen, World &world, Player &player) : lvlGen(lvlGen), world(world), player(player) {

    // Generate main map (TODO: Will be in loop and random maps later)
    lvlGen.GenerateFromImage(ASSET_DIR "/levels/map1.png", &world);
    // Wiil be handled by server later
    lvlGen.SetPlayerPosition(player);
}

void Game::UpdateOtherPlayers() {
    for (const auto& [name, settings] : Global::Get().players) {
        if (name == player.username) continue;

        if (!otherPlayers.contains(name)) {
            otherPlayers.emplace(name, Player(settings.position));
            otherPlayers[name].username = name;
        }

        otherPlayers[name].position = Vector2Lerp(otherPlayers[name].position, settings.position, 0.6f);
    }
}

int Game::Loop(NetworkClient *networkClient, CameraManager &camera_manager) {
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();
        // Update
        //----------------------------------------------------------------------------------
        player.GetInputs();

        world.Update(dt);
        player.Update(dt);

        player.CheckCollisions(world);
        if (networkClient)
        {
            // const std::string send_data = "3|" + player.username
            //                  + "|[" + std::to_string(player.position.x)
            //                  + ","
            //                  + std::to_string(player.position.y) + "]:";
            json send_data;
            send_data["name"] = player.username;
            send_data["px"] = std::to_string(player.position.x);
            send_data["py"] = std::to_string(player.position.y);
            networkClient->SendPacket(send_data.dump());
        }

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

        // end ImGui Content
        rlImGuiEnd();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    CloseWindow();        // Close window and OpenGL context

    return EXIT_SUCCESS;

}