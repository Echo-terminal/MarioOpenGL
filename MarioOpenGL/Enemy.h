#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Block.h"
#include <vector>

#include <stb_image.h>
#include <iostream>

class Enemy {
public:
    glm::vec2 position;
    glm::vec2 size = glm::vec2(32.0f, 32.0f);
    float speed;
    float speedY;
    float gravity;

    GLuint textureID;
    bool onGround;

    // Параметры для движения туда-сюда
    float moveDirection; // 1.0f = вправо, -1.0f = влево

    bool isSquashed = false;
    float squashTimer = 0.0f;
    float squashDuration = 1.5f; // время до исчезновения
    float originalHeight = 32.0f;
    float squashedHeight = 8.0f; // высота сплющенного врага

    Enemy();
    Enemy(glm::vec2 startPos);

    void Squash(); // новый метод
    bool ShouldBeRemoved() const; // новый метод

    void Update(float deltaTime, const std::vector<Block>& blocks);
    void Move(float deltaTime);
    void Falling(float deltaTime);
    void CheckBlockCollisions(const std::vector<Block>& blocks);

    bool LoadTexture(const char* path, GLuint& textureID);
    void Render(GLuint shaderProgram, const glm::mat4& projection);

    // Проверка коллизии с блоками (для поворота при столкновении со стеной)
    Collision CheckCollision(const Block& block) const;

private:
    bool initialized;
    GLuint VAO, VBO, EBO;
    void InitRenderData();
};