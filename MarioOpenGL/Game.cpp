#include "Game.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//=====================================================================================



// ====================================================================================

Game::Game(unsigned int width, unsigned int height)
    : screenWidth(width), screenHeight(height),
    window(nullptr), shaderProgram(0),
    cameraX(0.0f), cameraY(0.0f) {}

Game::~Game() {
    CleanUp();
}

bool Game::Init() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(screenWidth, screenHeight, "Mario Game", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return false;
    }

    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaderProgram = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    if (shaderProgram == 0) {
        std::cerr << "Shader compilation/linking failed\n";
        return false;
    }

    if (!player.LoadTexture("player.png", player.textureID)) {
        std::cerr << "Player texture load failed\n";
        return false;
    }
    // проверОчка на текстуры
    for (int i = 0; i < 5; i++)
    {
        if (!block[i].LoadTexture("block.png", block[i].textureID))
        {
            std::cerr << "Block texture load failed\n";
            return false;
        }
    }
    

    // Инициализируем позицию игрока в "мире"
    player.position = glm::vec2(400.0f, 300.0f); // Стартовая позиция в мире
    // Инициализируем камеру - центрируем на игроке
    UpdateCamera();

    return true;
}

void Game::ProcessInput(float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        player.Move(-deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        player.Move(deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !player.isJumping) {
        player.isJumping = true;
    }
}

void Game::Update(float deltaTime) {
    // Обновляем физику игрока
    if (player.isJumping) {
        player.position.y -= player.jumpSpeed * deltaTime * 100.0f;
        if (player.position.y <= 100.0f) {
            player.position.y = 100.0f;
            player.isJumping = false;
        }
    }
    
    //чекаем жестко колизию
    for (int i = 0; i < 5; ++i) {
        Collision info = player.CheckCollisionWith(block[i]);

        if (info.isColliding) {
            std::cout << "Collision with block " << i << " from: " << info.side << "\n";

           //перемещаем нарушителя колизийного покоя 
           //в зависимости от того с какой стороны он 
            if (info.side == "bottom" || info.side == "top")
                player.position.y = block[i].position.y - player.size.y;
            if (info.side == "right")
                player.position.x = block[i].position.x - player.size.x;
            else if (info.side == "left")
                player.position.x = block[i].position.x + player.size.x;
            
            
            
        }
    }

    // Обновляем камеру после движения игрока
    UpdateCamera();
}

void Game::UpdateCamera() {
    // Центрируем камеру на игроке
    cameraX = player.position.x - screenWidth / 2.0f;
    cameraY = player.position.y - screenHeight / 2.0f;

    // Можно добавить ограничения камеры, например:
    // if (cameraX < 0) cameraX = 0;
    // if (cameraY < 0) cameraY = 0;

    // Создаем новую матрицу проекции с учетом позиции камеры
    projection = glm::ortho(
        cameraX, cameraX + screenWidth,     // left, right
        cameraY + screenHeight, cameraY,    // bottom, top
        -1.0f, 1.0f                         // near, far
    );
}

void Game::Render() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Передаем обновленную матрицу проекции (с камерой) в шейдер
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    player.Render(shaderProgram, projection);
   

    // Здесь можно рендерить другие объекты мира
    RenderWorld();
}



void Game::RenderWorld() {
    block[0].position = glm::vec2(400.0f, 332.0f);
    block[1].position = glm::vec2(432.0f, 300.0f);
    block[2].position = glm::vec2(432.0f, 268.0f);
    block[3].position = glm::vec2(368.0f, 300.0f);
    block[0].Render(shaderProgram, projection);
    block[1].Render(shaderProgram, projection);
    block[2].Render(shaderProgram, projection);
    block[3].Render(shaderProgram, projection);
    //пока так рендерим мир 
}

glm::vec2 Game::WorldToScreen(const glm::vec2& worldPos) {
    // Преобразование мировых координат в экранные
    return glm::vec2(worldPos.x - cameraX, worldPos.y - cameraY);
}

glm::vec2 Game::ScreenToWorld(const glm::vec2& screenPos) {
    // Преобразование экранных координат в мировые
    return glm::vec2(screenPos.x + cameraX, screenPos.y + cameraY);
}

void Game::CleanUp() {
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    if (player.textureID != 0) {
        glDeleteTextures(1, &player.textureID);
        player.textureID = 0;
    }
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

GLuint Game::LoadShaders(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (...) {
        std::cerr << "Failed to read shader files\n";
        return 0;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLint success;
    GLchar infoLog[512];
    GLuint vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "Vertex shader error:\n" << infoLog << std::endl;
        return 0;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "Fragment shader error:\n" << infoLog << std::endl;
        return 0;
    }

    GLuint ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cerr << "Shader program link error:\n" << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return ID;
}


