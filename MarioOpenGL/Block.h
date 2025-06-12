#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>
#include <iostream>

struct Collision
{
    bool isColliding = false;
    std::string side; // "top", "bottom", "left", "right"
};

class Block {
public:
    glm::vec2 position;
    GLuint textureID;
    glm::vec2 size = glm::vec2(32.0f, 32.0f);
    char blockType;
        
    //для buf | dvorax
    bool isBuf;
    glm::vec2 oldPos;
    bool isJump;
    float jumpSpeed = 150.0;
    bool withPowerUp;

    Block();
    std::string getTex(char symbol);
    bool LoadTexture(const char* path, GLuint& textureID);
    void Render(GLuint shaderProgram, const glm::mat4& projection);
    void Buf(float deltaTime);

private:
    bool initialized;
    GLuint VAO, VBO, EBO;
    void InitRenderData();
};




