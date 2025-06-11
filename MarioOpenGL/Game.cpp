#include "Game.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ====================================================================================

Game::Game(unsigned int width, unsigned int height)
    : screenWidth(width), screenHeight(height),
    window(nullptr), shaderProgram(0),
    cameraX(0.0f), cameraY(-280.202f) {}
 //                   ^насчёт этого читай ниже | dvorax

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
    if (!loadLvl("lvl.txt")) return false;
    
    
    
    UpdateCamera();

    return true;
}

void Game::ProcessInput(float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        player.Move(-deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        player.Move(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        player.position = glm::vec2(0.0f);
        //player.position = PlayerStartPos
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && player.onGround) {
        player.speedY = -300.0f;
        player.onGround = false;
    }
}

void Game::Update(float deltaTime) {
    // Обновляем физику игрока
    player.Falling(deltaTime);
    
    bool isOnGround = false;

    //чекаем жестко колизию | dvorax
    //теперь ещё мы тут и проверяем на buf | dvorax
    for (int i = 0; i < blocks.size(); i++) 
    {
        Collision info = player.CheckCollisionWithBlock(blocks[i]);
        blocks[i].Buf(deltaTime);

        if (info.isColliding) {
            std::cout << "Collision with block " << i << " from: " << info.side << "\n";

            // Отметим, что игрок стоит на земле, только если столкновение снизу
            if (info.side == "bottom") {
                isOnGround = true;
                player.speedY = 0;
            }

            // Разруливаем столкновение
            if (info.side == "top") {
                player.position.y = blocks[i].position.y + 32.0f; // высота блока
                blocks[i].isBuf = true;
            }
            else if (info.side == "bottom") {
               
                player.position.y = blocks[i].position.y - player.size.y;
            }
            else if (info.side == "right") {
                player.position.x = blocks[i].position.x - player.size.x;
            }
            else if (info.side == "left") {
                player.position.x = blocks[i].position.x + 32.0f; // ширина блока
            }
        }
    }

    player.onGround = isOnGround;


    // Обновляем камеру после движения игрока
    UpdateCamera();
}

void Game::UpdateCamera() {
    cameraX = player.position.x - screenWidth / 2.0f;

    //по Y нам не надо камеру двигать, в оригинале было так 
    // число то что верху это то значение которое на старте высчитывается | dvorax
    //cameraY = player.position.y - screenHeight / 1.19f; 
    
   
    // Можно добавить ограничения камеры, например:
     if (cameraX < 0) cameraX = 0;
    
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

bool Game::loadLvl(const std::string& path)
{
    std::ifstream lvl(path);
    if (!lvl.is_open())
    {
        std::cerr << "Failed to load level: " << path << std::endl;
        return false;
    }
    std::string lvlLine;
    int row = 0;

    while (std::getline(lvl, lvlLine))
    {

        for (int i = 0; i < lvlLine.size(); i++)
        {
            char symbol = lvlLine[i];
            if (symbol == '.') continue;
            if (symbol == 'P')
            {
                PlayerStartPos = glm::vec2(i * 32, row * 32);
                player.position = PlayerStartPos;
                continue;
            }
            Block block;
            block.position = glm::vec2(i * 32, row * 32);
            if (!block.LoadTexture(block.getTex(symbol).c_str(), block.textureID))
                return false;
            blocks.push_back(block);
        }
        row++;
    }
    return true;
}

void Game::RenderWorld() {
    for (Block& block : blocks) 
        block.Render(shaderProgram, projection);
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


