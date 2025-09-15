//
// Created by kshitij on 14/9/25.
//

#include "Bullet.hpp"
#include "raymath.h"

Bullet::Bullet(float speed) {
    this->speed = speed;
}

void Bullet::SetAngle(const float angle) {
    velocity.x = speed * cos(angle);
    velocity.y = speed * sin(angle);
}

void Bullet::SetActive() {
    active = true;
    lifeCtr = 0;
}

void Bullet::Update(float dt) {
    if (!active) return;

    position += velocity * dt;
    lifeCtr += dt;
    if (lifeCtr > lifetime) {
        active = false;
    }
}

void Bullet::Draw() {
    if (!active) return;
    DrawCircle(position.x, position.y, 10, BLACK);
    DrawCircle(position.x, position.y, 8, RED);
}