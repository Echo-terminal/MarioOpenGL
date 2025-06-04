#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>   // “ŒÀ‹ Œ Ú‡Í ó ·ÂÁ STB_IMAGE_IMPLEMENTATION
#include <iostream>

class Player {
public:
    glm::vec2 position;
    float speed;
    float jumpSpeed;
    bool isJumping;
    GLuint textureID;

    Player();
    void Move(float change);
    bool LoadTexture(const char* path, GLuint& textureID);
    void Render(GLuint shaderProgram, const glm::mat4& projection);

private:
    bool initialized;
    GLuint VAO, VBO, EBO;
    void InitRenderData();
};
