//
// Created by kshitij on 9/9/25.
//

#include "GameObject.hpp"

#include <iostream>
#include <bits/ostream.tcc>

#include "raymath.h"

void GameObject::Update(float dt) {
    velocity += acceleration;
    position += velocity * dt;
}

void GameObject::Draw() {
    std::cout << "This is the default draw, override please" << std::endl;
}