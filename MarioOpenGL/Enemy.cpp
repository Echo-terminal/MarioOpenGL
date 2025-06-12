#include "Enemy.h"

Enemy::Enemy()
    : position(0.0f, 0.0f), speed(80.0f), gravity(900.0f),
    speedY(0.0f), onGround(false), moveDirection(-1.0f),
    initialized(false), VAO(0), VBO(0), EBO(0),
    isSquashed(false), squashTimer(0.0f), squashDuration(1.5f),
    originalHeight(32.0f), squashedHeight(8.0f) {}

Enemy::Enemy(glm::vec2 startPos)
    : position(startPos), speed(80.0f), gravity(900.0f),
    speedY(0.0f), onGround(false), moveDirection(-1.0f),
    initialized(false), VAO(0), VBO(0), EBO(0),
    isSquashed(false), squashTimer(0.0f), squashDuration(1.5f),
    originalHeight(32.0f), squashedHeight(8.0f) {}

void Enemy::Update(float deltaTime, const std::vector<Block>& blocks) {
    if (isSquashed) {
        squashTimer += deltaTime;
        return; // сплющенный враг не двигаетс€
    }
    
    Move(deltaTime);
    Falling(deltaTime);
    CheckBlockCollisions(blocks);
}

void Enemy::Move(float deltaTime) {
    position.x += speed * moveDirection * deltaTime;
}

void Enemy::Falling(float deltaTime) {
    if (onGround) return;

    speedY += gravity * deltaTime;
    position.y += speedY * deltaTime;
}

void Enemy::CheckBlockCollisions(const std::vector<Block>& blocks) {
    // —начала предполагаем, что враг не на земле
    onGround = false;

    for (const auto& block : blocks) {
        Collision collision = CheckCollision(block);

        if (collision.isColliding) {
            if (collision.side == "bottom") {
                // ¬раг стоит на блоке
                position.y = block.position.y - size.y;
                speedY = 0.0f;
                onGround = true;
            }
            else if (collision.side == "top") {
                // ¬раг ударилс€ головой о блок
                position.y = block.position.y + 32.0f;
                speedY = 0.0f;
            }
            else if (collision.side == "left") {
                // ¬раг ударилс€ о блок слева (враг справа от блока)
                position.x = block.position.x + 32.0f;
                moveDirection = 1.0f; // поворачиваем вправо
            }
            else if (collision.side == "right") {
                // ¬раг ударилс€ о блок справа (враг слева от блока)
                position.x = block.position.x - size.x;
                moveDirection = -1.0f; // поворачиваем влево
            }
        }
    }
}

Collision Enemy::CheckCollision(const Block& block) const {
    Collision result;
    glm::vec2 posA = position;
    glm::vec2 sizeA = size;
    glm::vec2 posB = block.position;
    glm::vec2 sizeB = glm::vec2(32.0f, 32.0f); // фиксированный размер блока

    bool xOverlap = posA.x < posB.x + sizeB.x && posA.x + sizeA.x > posB.x;
    bool yOverlap = posA.y < posB.y + sizeB.y && posA.y + sizeA.y > posB.y;

    // ƒобавим небольшой допуск дл€ сравнени€ (float-precision)
    const float epsilon = 0.001f;

    // ѕроверка на то, что враг стоит точно на блоке (по оси Y)
    bool standingOnBlock =
        xOverlap &&
        std::abs((posA.y + sizeA.y) - posB.y) < epsilon;

    if ((xOverlap && yOverlap) || standingOnBlock) {
        result.isColliding = true;

        float deltaRight = (posA.x + sizeA.x) - posB.x;
        float deltaLeft = (posB.x + sizeB.x) - posA.x;
        float deltaBottom = (posA.y + sizeA.y) - posB.y;
        float deltaTop = (posB.y + sizeB.y) - posA.y;

        float minX = std::min(deltaRight, deltaLeft);
        float minY = std::min(deltaBottom, deltaTop);

        if (standingOnBlock) {
            result.side = "bottom";
        }
        else if (minX < minY) {
            result.side = (deltaRight < deltaLeft) ? "right" : "left";
        }
        else {
            result.side = (deltaBottom < deltaTop) ? "bottom" : "top";
        }
    }

    return result;
}

bool Enemy::LoadTexture(const char* path, GLuint& textureID) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return false;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return true;
}

void Enemy::Render(GLuint shaderProgram, const glm::mat4& projection) {
    if (!initialized) InitRenderData();

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));

    // ћасштабируем спрайт в зависимости от состо€ни€
    glm::vec3 scale(1.0f, size.y / originalHeight, 1.0f);

    // ќтражаем спрайт в зависимости от направлени€ движени€
    if (moveDirection < 0) {
        scale.x = -scale.x;
        model = glm::translate(model, glm::vec3(size.x, 0.0f, 0.0f));
    }

    model = glm::scale(model, scale);

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Enemy::InitRenderData() {
    float vertices[] = {
        // pos      // tex
         0.0f,  0.0f,  0.0f, 0.0f,
        32.0f,  0.0f,  1.0f, 0.0f,
        32.0f, 32.0f,  1.0f, 1.0f,
         0.0f, 32.0f,  0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texCoords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    initialized = true;
}

void Enemy::Squash() {
    if (!isSquashed) {
        isSquashed = true;
        squashTimer = 0.0f;
        size.y = squashedHeight;
        speedY = 0.0f;
        speed = 0.0f; // останавливаем движение

        //  орректируем позицию, чтобы враг "прижалс€" к земле
        position.y += (originalHeight - squashedHeight);

        std::cout << "Enemy squashed!\n";
    }
}

bool Enemy::ShouldBeRemoved() const {
    return isSquashed && squashTimer >= squashDuration;
}
