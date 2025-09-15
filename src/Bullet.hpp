//
// Created by kshitij on 14/9/25.
//

#pragma once
#include "GameObject.hpp"

class Bullet : public GameObject {
public:
    Bullet(float speed=1200);

    float speed;
    bool active = false;
    float lifetime = 3.0f;
    float lifeCtr = 0;

    void SetAngle(float angle);
    void SetActive();

    void Update(float dt) override;
    void Draw() override;
};
