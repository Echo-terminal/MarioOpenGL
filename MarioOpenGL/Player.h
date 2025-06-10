#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Block.h"

#include <stb_image.h>   // “ŒÀ‹ Œ Ú‡Í ó ·ÂÁ STB_IMAGE_IMPLEMENTATION
#include <iostream>


struct Collision
{
    bool isColliding = false;
    std::string side; // "top", "bottom", "left", "right"
};


class Player {
public:
    glm::vec2 position;
    float speed;
    float jumpSpeed;
    bool isJumping;
    GLuint textureID;
    glm::vec2 size = glm::vec2(32.0f, 32.0f);

    Player();
    void Move(float change);
    bool LoadTexture(const char* path, GLuint& textureID);
    void Render(GLuint shaderProgram, const glm::mat4& projection);
    Collision CheckCollisionWith(const Block& block) const;

private:
    bool initialized;
    GLuint VAO, VBO, EBO;
    void InitRenderData();
};


