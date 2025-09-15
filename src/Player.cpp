//
// Created by kshitij on 06/09/25.
//

#include "Player.hpp"

#include <string>

#include "World.hpp"
#include <algorithm>

#include "AssetLoader.hpp"
#include "Bullet.hpp"
#include "Constants.hpp"

Player::Player(Vector2 pos) {
    pos -= size/2;
    position = pos;

    jumpBufferCounter = 0;
    hangTimeCtr = 0;

    velocity = Vector2{0, 0};
    acceleration = Vector2{0, 9.8f * 5};

    // Bullets
    bullets = std::vector<Bullet>(maxBullets);

    collisionShape = Rectangle(pos.x, pos.y, size.x, size.y);

    texture = AssetLoader::Get().textures["Player_Head"];
    sourceRect = Rectangle{0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height)};
}

void Player::GetInputs() {
    horizontal.x = IsKeyDown(KEY_A) ? -1 : IsKeyDown(KEY_D) ? 1 : 0;

    if (IsKeyReleased(KEY_SPACE) && velocity.y < 0) {
        velocity.y = velocity.y * jumpStopFactor;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Shoot();
    }
}

void Player::Shoot() {
    for (auto &b : bullets) {
        if (!b.active) {
            b.SetActive();
            b.position = shootPoint;
            b.SetAngle(angle);
            return;
        }
    }
}

void Player::Jump(float dt) {
    velocity.y = -jumpHeight;
}

void Player::CheckCollisions(const World &world) {
    bool colliding = false;

    Rectangle prevCollisionShape = {
        position.x - velocity.x,
        position.y - velocity.y,
        size.x,
        size.y
    };

    for (const auto &platform : world.platforms) {
        if (CheckCollisionRecs(platform.collisionShape, collisionShape)) {
            colliding = true;

            float overlapLeft   = (collisionShape.x + collisionShape.width) - platform.collisionShape.x;
            float overlapRight  = (platform.collisionShape.x + platform.collisionShape.width) - collisionShape.x;
            float overlapTop    = (collisionShape.y + collisionShape.height) - platform.collisionShape.y;
            float overlapBottom = (platform.collisionShape.y + platform.collisionShape.height) - collisionShape.y;

            // Find smallest overlap → that’s the direction of resolution
            float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});

            if (minOverlap == overlapTop && prevCollisionShape.y + prevCollisionShape.height <= platform.collisionShape.y) {
                // Landed on top
                position.y = platform.position.y - size.y;
                velocity.y = 0;
                isGrounded = true;
            }
            else if (minOverlap == overlapBottom && prevCollisionShape.y >= platform.collisionShape.y + platform.collisionShape.height) {
                // Hit from below
                position.y = platform.position.y + platform.size.y;
                velocity.y = 0; // maybe bounce or stop
            }
            else if (minOverlap == overlapLeft && prevCollisionShape.x + prevCollisionShape.width <= platform.collisionShape.x) {
                // Hit from left
                position.x = platform.position.x - size.x;
                velocity.x = 0;
            }
            else if (minOverlap == overlapRight && prevCollisionShape.x >= platform.collisionShape.x + platform.size.x) {
                // Hit from right
                position.x = platform.position.x + platform.size.x;
                velocity.x = 0;
            }
        }
    }

    if (!colliding) {
        isGrounded = false;
    }
}

void Player::Update(float dt) {
    velocity.x = (horizontal.x * speed * dt);
    // Gravity
    velocity += acceleration * dt;

    position += velocity;
    // Simple Ground

    collisionShape.x = position.x;
    collisionShape.y = position.y;

    // Coyote time
    if (isGrounded) hangTimeCtr = hangTime;
    else hangTimeCtr -= dt;

    if (IsKeyPressed(KEY_SPACE)) jumpBufferCounter = jumpBufferTime;
    else jumpBufferCounter -= dt;

    if (jumpBufferCounter >= 0 && hangTimeCtr > 0) {
        _canJump = true;
        jumpBufferCounter = 0;
    }

    if (_canJump) {
        Jump(dt);
        _canJump = false;
        isGrounded = false;
    }

    // Bullets
    for (auto &b : bullets) {
        b.Update(dt);
    }

}

void Player::Draw() {
    // Draw text above head ----------------------
    const int textWidth = MeasureText(username.data(), 18);
    const int posX = position.x + (size.x/2) - (textWidth / 2);

    DrawText(username.data(), posX, position.y - 30, 18, BLACK);
    // -------------------------------------------
    headPosition.x = position.x + (size.x/2);
    headPosition.y = position.y + (size.y/4);


    DrawRectangleRounded(
        Rectangle{ position.x, position.y + size.y/2,size.x, size.y/2},
        0.2,
        10,
        BLACK);
    DrawRectangleRounded(
        Rectangle{ position.x + 2, position.y + size.y/2 + 2,size.x - 4, size.y/2 - 4},
        0.2,
        10,
        RED);

    // Draw ShootPoint
    DrawCircle(shootPoint.x, shootPoint.y, 4, GREEN);
    // Draw Head
    float scaling = 1.7;
    auto newSize = Vector2(size.x * scaling, size.x * scaling);
    auto destRect = Rectangle{headPosition.x, headPosition.y , newSize.x, newSize.y};
    DrawTexturePro(texture, sourceRect, destRect, newSize/2, angle * RAD2DEG - 90, WHITE);

    for (auto &b : bullets) {
        b.Draw();
    }
}

void Player::Draw(Vector2* mousePosition) {
    headPosition.x = position.x + (size.x/2);
    headPosition.y = position.y + (size.y/4);

    const float dx = mousePosition->x - headPosition.x;
    const float dy = mousePosition->y - headPosition.y;

    angle = atan2f(dy, dx);

    const float dxx = (20) * cosf(angle);
    const float dyy = (20) * sinf(angle);

    shootPoint.x = headPosition.x + dxx;
    shootPoint.y = headPosition.y + dyy;

    Draw();
}

Player::~Player() {
    // UnloadTexture(texture);
}
