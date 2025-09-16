//
// Created by kshitij on 06/09/25.
//
#include <string>

#include "GameObject.hpp"
#include "raylib.h"
#include "raymath.h"
#include "World.hpp"
#include "../../../../../../home/kshitij/projects/cpp/Platformer_RL/src/Bullet.hpp"

#ifndef PLATFORMER_RL_PLAYER_HPP
#define PLATFORMER_RL_PLAYER_HPP

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Player : public GameObject{
public:
    // Display
    Vector2 size = Vector2(30, 60);
    Vector2 headPosition = Vector2(0, 0);
    Vector2 shootPoint = Vector2(0, 0);
    float angle = 90 * DEG2RAD;
    Rectangle sourceRect = Rectangle(0, 0, 32, 32);

    bool isGrounded = false;
    std::string username = "Test Player";

    // Visual
    Texture2D texture{};

    // Movement
    Vector2 horizontal{};
    float speed = 450;
    float jumpHeight = 18;
    float jumpStopFactor = 0.3f;
    float sprintMultiplier = 1.7f;
    bool _canJump = false;

    // Better Platformer
    float hangTime = 0.1f;
    float hangTimeCtr;

    float jumpBufferTime = 0.1f;
    float jumpBufferCounter;

    // Shooting
    const int maxBullets = 100;
    std::vector<Bullet> bullets;

    Player(Vector2 pos);
    Player() : Player(Vector2(0, 0)) {}
    ~Player();

    void GetInputs();
    void Update(float dt) override;
    void Draw(Vector2* mousePosition);
    void Draw() override;
    void Shoot();

    void Jump(float dt);

    void CheckCollisions(const World &world);
};

// Defining how to convert from json to Player (and vv)
namespace nlohmann {
    template<>
    struct nlohmann::adl_serializer<Player> {
        static void to_json(json &j, const Player &p) {
            j = json{
                {"name", p.username},
                {"px", p.position.x},
                {"py", p.position.y},
                {"angle", p.angle}
            };
        }

        static void from_json(const json &j, Player &p) {
            try {
                p.username = j.at("name").get<std::string>();
                p.position.x = j.at("px").get<float>();
                p.position.y = j.at("py").get<float>();
                p.angle = j.at("angle").get<float>();
            } catch (const std::exception &e) {
                std::cout << "Invalid JSON Format: " << e.what() << std::endl;
            }
        }
    };
}

#endif //PLATFORMER_RL_PLAYER_HPP