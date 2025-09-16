//
// Created by kshitij on 12/9/25.
//
#pragma once
#include <raylib.h>
#include <string>
#include <unordered_map>

struct PlayerSettings {
    Vector2 position;
};

class Global {
public:
    // Remove copy constructor
    Global(const Global&) = delete;
    static Global& Get() {
        return instance;
    }

    std::unordered_map<std::string, PlayerSettings> players;
    bool allReady = false;

private:
    Global() {}

    static Global instance;
};