//
// Created by kshitij on 14/9/25.
//

#pragma once
#include <raylib.h>
#include <unordered_map>
#include <string>

class AssetLoader {
public:
    // Remove copy constructor
    AssetLoader(const AssetLoader&) = delete;

    static AssetLoader& Get() {
        return instance;
    }
    void LoadAssets();
    void UnloadAssets();

    std::unordered_map<std::string, Texture2D> textures;
private:
    AssetLoader() {}

    static AssetLoader instance;
};