#include <iostream>
#include <chrono>
#include "Game.h"

int main() {
    // Создаем игру с разрешением 800x600
    Game game(800, 600);

    // Инициализируем игру
    if (!game.Init()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }

    // Переменные для подсчета времени
    auto lastTime = std::chrono::high_resolution_clock::now();

    std::cout << "Game started! Use arrow keys to move, SPACE to jump, ESC to exit." << std::endl;

    // Основной игровой цикл
    while (!game.ShouldClose()) {
        // Подсчитываем deltaTime
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Ограничиваем deltaTime чтобы избежать больших скачков при лагах
        if (deltaTime > 0.016f) {
            deltaTime = 0.016f; // ~60 FPS
        }

        // Обрабатываем события GLFW
        game.PollEvents();

        // Обрабатываем ввод
        game.ProcessInput(deltaTime);

        // Обновляем игру
        game.Update(deltaTime);

        // Рендерим
        game.Render();

        // Меняем буферы
        game.SwapBuffers();
    }

    // Очистка происходит автоматически в деструкторе Game
    std::cout << "Game ended." << std::endl;
    return 0;
}