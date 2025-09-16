//
// Created by kshitij on 9/9/25.
//

#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP
#include <raylib.h>

class GameObject {
private:
    int health = 100;
public:
    // Basic
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    Vector2 acceleration = {0, 0};

    int GetHealth() { return health; }
    void SetHealth(int health) { this->health = health; }
    void Damage(int amount) { health -= amount; }

    Rectangle collisionShape;

    virtual void Update(float dt);

    virtual void Draw();
};

#endif //GAMEOBJECT_HPP
