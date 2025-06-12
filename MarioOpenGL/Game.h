#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>

// Предполагаем, что у вас есть класс Player
#include "Player.h"
#include "Block.h"
#include "Enemy.h"
#include "powerUp.h"

class Game {
public:
    Game(unsigned int width, unsigned int height);
    ~Game();

    bool Init();
    void ProcessInput(float deltaTime);
    void Update(float deltaTime);
    void Render();
    void CleanUp();
    bool loadLvl(const std::string& path);
    void restart();

    // Камера
    void UpdateCamera();
    glm::vec2 WorldToScreen(const glm::vec2& worldPos);
    glm::vec2 ScreenToWorld(const glm::vec2& screenPos);

    // Геттеры для камеры
    float GetCameraX() const { return cameraX; }
    float GetCameraY() const { return cameraY; }

    // Проверка, должно ли окно закрыться
    bool ShouldClose() { return glfwWindowShouldClose(window); }
    void SwapBuffers() { glfwSwapBuffers(window); }
    void PollEvents() { glfwPollEvents(); }

    // Геттер для окна
    GLFWwindow* GetWindow() const { return window; }

private:
    unsigned int screenWidth, screenHeight;
    GLFWwindow* window;
    GLuint shaderProgram;

    // Камера
    float cameraX, cameraY;
    glm::mat4 projection;

    // Игровые объекты
    Player player;
    std::vector<Block> blocks;
    std::vector<Enemy> enemies;
    std::vector<PowerUp> powerUps;

    glm::vec2 PlayerStartPos;

    std::string levelPath;


    // Вспомогательные функции
    GLuint LoadShaders(const char* vertexPath, const char* fragmentPath);
    void RenderWorld();
};