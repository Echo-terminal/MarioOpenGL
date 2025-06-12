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
        restart();
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && player.onGround) {
        player.speedY = -300.0f;
        player.onGround = false;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        player.ChangeSize();
    }
}

void Game::Update(float deltaTime) {
    // Обновляем физику игрока
    player.Falling(deltaTime);
    
    bool isOnGround = false;

    if (player.position.y > 300.0f) {
        restart();
        return;
    }

    if (player.position.x >= lvlWidth) {
        std::cout << "You won!" << std::endl;
        glfwSetWindowShouldClose(window, true);
        return;
    }

    if (player.imortal)
    {
        player.imortalTimer -= deltaTime;
        if (player.imortalTimer <= 0.0f)
        {
            player.imortal = false;
            player.imortalTimer = 0.0f;
        }
    }
    //чекаем жестко колизию | dvorax
    //теперь ещё мы тут и проверяем на buf | dvorax
    for (int i = 0; i < blocks.size(); i++) 
    {
        Collision info = player.CheckCollision(blocks[i]);
        blocks[i].Buf(deltaTime);

        if (info.isColliding) {
            //std::cout << "Collision with block " << i << " from: " << info.side << "\n";

            // Отметим, что игрок стоит на земле, только если столкновение снизу
            if (info.side == "bottom") {
                player.blockHit = false;
                isOnGround = true;
                player.speedY = 0;
            }

            // Разруливаем столкновение
            if (info.side == "top") {
                player.position.y = blocks[i].position.y + 32.0f; // высота блока
                if (blocks[i].blockType == 'B' || blocks[i].blockType == '?' 
                    || blocks[i].blockType == '-' || blocks[i].blockType == '+')
                {
                    if (!player.blockHit)
                    {
                        blocks[i].isBuf = true;
                        player.blockHit = true;

                        if (blocks[i].blockType == '+')
                        {
                            PowerUp newItem;
                            newItem.LoadTexture( "mush.png", newItem.textureID);
                            newItem.position = blocks[i].position - glm::vec2(0.0f, 32.0f);
                            powerUps.push_back(newItem);

                        }
                    }
                }
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

    for (int i = 0; i < enemies.size(); ++i) {
        enemies[i].Update(deltaTime, blocks);

        // Проверяем, нужно ли удалить врага
        if (enemies[i].ShouldBeRemoved()) {
            enemies.erase(enemies.begin() + i);
            --i;
            continue;
        }
        
        if (enemies[i].isSquashed) {
            continue;
        }

        Collision info = player.CheckCollisionEnemy(enemies[i]);
        if (info.isColliding) {
            //std::cout << "Collision with enemy " << i << " from: " << info.side << "\n";
            if (player.imortal)
            {
                std::cout << "Imortal " << "\n";
                continue;
            }
            if (info.side == "bottom") {
                // игрок приземлился сверху — «убиваем» врага
                enemies[i].Squash();

                // Добавляем отскок игрока (как в Марио)
                player.speedY = -200.0f;
            }
            else {
                // любая другая сторона — рестарт
                
                // ещё нет если мы большие то ещё живем | dvorax
                if (player.big)
                {
                    player.ChangeSize();
                    player.imortal = true;
                    player.imortalTimer = 3.0f;
                }
                else restart();
                return; // выходим, чтобы не продолжать апдейт после рестарта
            }
        }
    }
    for (int i = 0; i < powerUps.size(); i++)
    {
        Collision info = player.CheckCollisionEnemy(powerUps[i]);
        if (info.isColliding) 
        {
            if (!player.big) player.ChangeSize();;
            powerUps.erase(powerUps.begin() + i);
        }
    }

    player.onGround = isOnGround;

    for (Enemy& enemy : enemies) {
        enemy.Update(deltaTime, blocks);
    }

    for (int i = 0; i < powerUps.size(); i++) powerUps[i].Update(deltaTime, blocks);
    // Обновляем камеру после движения игрока
    UpdateCamera();
}

void Game::restart() {
    // 1. Сбросить состояние игрока
    player.position = PlayerStartPos;
    player.Reset();

    // 2. Очистить старые блоки, врагов и power-ups
    blocks.clear();
    enemies.clear();
    powerUps.clear();

    lvlWidth = 0.0f;

    // 3. Загрузить уровень заново
    if (!loadLvl(levelPath)) {
        std::cerr << "Failed to reload level during restart\n";
        return;
    }

    // 4. Обновить камеру, чтобы снова центрироваться на игроке
    UpdateCamera();
}

void Game::UpdateCamera() {
    cameraX = player.position.x - screenWidth / 2.0f;

    //по Y нам не надо камеру двигать, в оригинале было так 
    // число то что верху это то значение которое на старте высчитывается | dvorax
    
   
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
    levelPath = path;
    lvlWidth = 0.0f;
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
            if (symbol == 'E')
            {
                Enemy enemy(glm::vec2(i * 32, row * 32)); // границы ±5 блоков
                if (!enemy.LoadTexture("enemy.png", enemy.textureID))
                    return false;
                enemies.push_back(enemy);
                continue;
            }
            Block block;
            block.position = glm::vec2(i * 32, row * 32);
            if (block.position.x > lvlWidth) {
                lvlWidth = block.position.x;
            }
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
    for (Enemy& enemy : enemies)
        enemy.Render(shaderProgram, projection);
    for (int i = 0; i < powerUps.size(); i++) powerUps[i].Render(shaderProgram, projection);
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
    for (Enemy& enemy : enemies) {
        if (enemy.textureID != 0) {
            glDeleteTextures(1, &enemy.textureID);
            enemy.textureID = 0;
        }
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


