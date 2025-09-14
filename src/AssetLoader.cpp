//
// Created by kshitij on 14/9/25.
//

#include "AssetLoader.hpp"

#include <iostream>

void AssetLoader::LoadAssets() {
    textures["Player_Head"] = LoadTexture(ASSET_DIR "/player/head.png");
}

void AssetLoader::UnloadAssets() {
    for (auto t : textures) {
        UnloadTexture(t.second);
    }
}