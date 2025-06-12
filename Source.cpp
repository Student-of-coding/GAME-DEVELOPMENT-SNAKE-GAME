#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Constants
// ─────────────────────────────────────────────────────────────────────────────

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;
const float        BLOCK_SIZE = 20.f;
const unsigned int COLUMNS = WINDOW_WIDTH / static_cast<unsigned>(BLOCK_SIZE);
const unsigned int ROWS = WINDOW_HEIGHT / static_cast<unsigned>(BLOCK_SIZE);

const float BONUS_SPAWN_INTERVAL = 15.f;
const float BONUS_DURATION = 5.f;
const int   INITIAL_LIVES = 3;

// Colors
const sf::Color BG_COLOR1 = sf::Color(34, 139, 34); // ForestGreen
const sf::Color BG_COLOR2 = sf::Color(46, 160, 46); // lighter
const sf::Color OBSTACLE_COLOR = sf::Color::Red;           // bright red
const sf::Color BONUS_COLOR = sf::Color::Yellow;
const sf::Color BORDER_COLOR = sf::Color(105, 105, 105); // DimGray

// ─────────────────────────────────────────────────────────────────────────────
// Enums & Structs
// ─────────────────────────────────────────────────────────────────────────────

enum Direction { None, Up, Down, Left, Right };
enum GameState { MainMenu, LevelSelect, Playing, Paused, GameOver };

struct Button {
    sf::RectangleShape box;
    sf::Text           label;
    Button(const sf::Font& font, const sf::String& str, unsigned charSize)
        : label(font, str, charSize)
    {
    }
    bool contains(const sf::Vector2i& m) const {
        return box.getGlobalBounds()
            .contains(sf::Vector2f{ float(m.x), float(m.y) });
    }
};

static void centerText(sf::Text& txt, float x, float y) {
    auto b = txt.getLocalBounds();
    txt.setOrigin({ b.size.x / 2.f, b.size.y / 2.f });
    txt.setPosition({ x, y });
}

// ─────────────────────────────────────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // 1) Load font & sounds via SFML 3 constructors
    sf::Font font;
    try {
        font = sf::Font("arial.ttf");
    }
    catch (...) {
        std::cerr << "Error: could not load arial.ttf\n";
        return EXIT_FAILURE;
    }

    sf::SoundBuffer gameOverBuf, eatBuf;
    try {
        gameOverBuf = sf::SoundBuffer("gameover.wav");
    }
    catch (...) {
        std::cerr << "Warning: could not load gameover.wav\n";
    }
    try {
        eatBuf = sf::SoundBuffer("eat.wav");
    }
    catch (...) {
        std::cerr << "Warning: could not load eat.wav\n";
    }
    sf::Sound gameOverSound(gameOverBuf);
    sf::Sound eatSound(eatBuf);

    // 2) Create window (SFML 3)
    sf::VideoMode vm{ sf::Vector2u{ WINDOW_WIDTH, WINDOW_HEIGHT } };
    sf::RenderWindow window(vm, "Snake Game");
    window.setFramerateLimit(60);

    // 3) Game variables
    std::vector<sf::Vector2f> snake;
    Direction dir = None;
    sf::Clock moveClock;
    float    moveDelay = 0.20f;

    sf::Vector2u foodCell{ std::rand() % COLUMNS, std::rand() % ROWS };
    sf::Vector2u bonusCell{ 0, 0 };
    bool         bonusActive = false;
    sf::Clock    bonusSpawnClock, bonusLiveClock;

    int  score = 0;
    int  level = 1;
    int  lives = INITIAL_LIVES;
    int  highScore = 0;
    int  startingLevel = 1;
    int  nextLevelScore = 100;  // Score needed to reach next level

    std::vector<sf::Vector2u> obstacles;

    // 4) Pre-create border shapes
    sf::RectangleShape borderTop, borderBottom, borderLeft, borderRight;
    borderTop.setSize({ float(WINDOW_WIDTH), BLOCK_SIZE });
    borderTop.setPosition({ 0.f, 0.f });
    borderTop.setFillColor(BORDER_COLOR);

    borderBottom.setSize({ float(WINDOW_WIDTH), BLOCK_SIZE });
    borderBottom.setPosition({ 0.f, float(WINDOW_HEIGHT - BLOCK_SIZE) });
    borderBottom.setFillColor(BORDER_COLOR);

    borderLeft.setSize({ BLOCK_SIZE, float(WINDOW_HEIGHT) });
    borderLeft.setPosition({ 0.f, 0.f });
    borderLeft.setFillColor(BORDER_COLOR);

    borderRight.setSize({ BLOCK_SIZE, float(WINDOW_HEIGHT) });
    borderRight.setPosition({ float(WINDOW_WIDTH - BLOCK_SIZE), 0.f });
    borderRight.setFillColor(BORDER_COLOR);

    sf::Text infoText(font, "", 20);
    infoText.setFillColor(sf::Color::White);
    infoText.setPosition({ BLOCK_SIZE + 5.f, BLOCK_SIZE + 5.f });

    sf::Text gameOverText(font, "Game Over!", 48);
    gameOverText.setFillColor(sf::Color::Red);
    centerText(gameOverText, WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f - 50.f);

    sf::Text pauseText(font, "Paused\nPress P to Resume\nPress M for Menu", 32);
    pauseText.setFillColor(sf::Color::Black);
    centerText(pauseText, WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

    // 5) Main Menu & LevelSelect UI
    sf::Text titleText(font, "SNAKE GAME", 50);
    titleText.setFillColor(sf::Color::Black);
    centerText(titleText, WINDOW_WIDTH / 2.f, 100.f);

    sf::Text highScoreText(font, "", 24);
    highScoreText.setFillColor(sf::Color::Black);
    centerText(highScoreText, WINDOW_WIDTH / 2.f, 160.f);

    Button playButton(font, "Play", 24);
    playButton.box.setSize({ 200.f, 50.f });
    playButton.box.setFillColor({ 200,200,200 });
    playButton.box.setPosition({ WINDOW_WIDTH / 2.f - 100.f, 200.f });
    centerText(playButton.label,
        playButton.box.getPosition().x + playButton.box.getSize().x / 2.f,
        playButton.box.getPosition().y + playButton.box.getSize().y / 2.f);

    Button exitButton(font, "Exit", 24);
    exitButton.box.setSize({ 200.f, 50.f });
    exitButton.box.setFillColor({ 200,200,200 });
    exitButton.box.setPosition({ WINDOW_WIDTH / 2.f - 100.f, 270.f });
    centerText(exitButton.label,
        exitButton.box.getPosition().x + exitButton.box.getSize().x / 2.f,
        exitButton.box.getPosition().y + exitButton.box.getSize().y / 2.f);

    std::vector<Button> levelButtons;
    for (int i = 1; i <= 5; ++i) {
        Button b(font, "Level " + std::to_string(i), 20);
        b.box.setSize({ 100.f, 40.f });
        b.box.setFillColor({ 180,180,180 });
        b.box.setPosition({ 150.f + (i - 1) * 110.f, 200.f });
        centerText(b.label,
            b.box.getPosition().x + b.box.getSize().x / 2.f,
            b.box.getPosition().y + b.box.getSize().y / 2.f);
        levelButtons.push_back(b);
    }
    Button backButton(font, "Back", 20);
    backButton.box.setSize({ 100.f, 40.f });
    backButton.box.setFillColor({ 180,180,180 });
    backButton.box.setPosition({ WINDOW_WIDTH / 2.f - 50.f, 300.f });
    centerText(backButton.label,
        backButton.box.getPosition().x + backButton.box.getSize().x / 2.f,
        backButton.box.getPosition().y + backButton.box.getSize().y / 2.f);

    // Game Over UI
    sf::Text gameOverTitle(font, "Game Over!", 60);
    gameOverTitle.setFillColor(sf::Color::Red);
    centerText(gameOverTitle, WINDOW_WIDTH / 2.f, 100.f);

    sf::Text finalScoreText(font, "", 30);
    finalScoreText.setFillColor(sf::Color::White);
    centerText(finalScoreText, WINDOW_WIDTH / 2.f, 180.f);

    sf::Text gameOverHighScoreText(font, "", 30);
    gameOverHighScoreText.setFillColor(sf::Color::White);
    centerText(gameOverHighScoreText, WINDOW_WIDTH / 2.f, 220.f);

    Button retryButton(font, "Retry", 24);
    retryButton.box.setSize({ 200.f, 50.f });
    retryButton.box.setFillColor(sf::Color(200, 200, 200));
    retryButton.box.setPosition({ WINDOW_WIDTH / 2.f - 100.f, 280.f });
    centerText(retryButton.label,
        retryButton.box.getPosition().x + retryButton.box.getSize().x / 2.f,
        retryButton.box.getPosition().y + retryButton.box.getSize().y / 2.f);

    Button menuButton(font, "Main Menu", 24);
    menuButton.box.setSize({ 200.f, 50.f });
    menuButton.box.setFillColor(sf::Color(200, 200, 200));
    menuButton.box.setPosition({ WINDOW_WIDTH / 2.f - 100.f, 350.f });
    centerText(menuButton.label,
        menuButton.box.getPosition().x + menuButton.box.getSize().x / 2.f,
        menuButton.box.getPosition().y + menuButton.box.getSize().y / 2.f);

    Button exitButtonGameOver(font, "Exit", 24);
    exitButtonGameOver.box.setSize({ 200.f, 50.f });
    exitButtonGameOver.box.setFillColor(sf::Color(200, 200, 200));
    exitButtonGameOver.box.setPosition({ WINDOW_WIDTH / 2.f - 100.f, 420.f });
    centerText(exitButtonGameOver.label,
        exitButtonGameOver.box.getPosition().x + exitButtonGameOver.box.getSize().x / 2.f,
        exitButtonGameOver.box.getPosition().y + exitButtonGameOver.box.getSize().y / 2.f);

    // 6) Helper lambdas
    auto isCellFree = [&](const sf::Vector2u& cell) -> bool {
        // Check obstacles
        for (const auto& o : obstacles) {
            if (o == cell) return false;
        }

        // Check snake
        for (const auto& seg : snake) {
            sf::Vector2u segCell(
                static_cast<unsigned>(seg.x / BLOCK_SIZE),
                static_cast<unsigned>(seg.y / BLOCK_SIZE)
            );
            if (segCell == cell) return false;
        }

        // Check bonus
        if (bonusActive && cell == bonusCell) return false;

        return true;
        };

    auto spawnFood = [&]() {
        std::vector<sf::Vector2u> freeCells;

        // Collect all free cells
        for (unsigned y = 1; y < ROWS - 1; ++y) {
            for (unsigned x = 1; x < COLUMNS - 1; ++x) {
                sf::Vector2u cell(x, y);
                if (isCellFree(cell)) {
                    freeCells.push_back(cell);
                }
            }
        }

        // If free cells available, pick random one
        if (!freeCells.empty()) {
            foodCell = freeCells[std::rand() % freeCells.size()];
        }
        else {
            // Fallback if no free cells (shouldn't normally happen)
            foodCell = { 1, 1 };
        }
        };

    auto spawnObstacles = [&](int lvl) {
        obstacles.clear();
        int count = std::min((lvl - 1) * 2, 40);
        while ((int)obstacles.size() < count) {
            sf::Vector2u cell{
                unsigned(1 + std::rand() % (COLUMNS - 2)),
                unsigned(1 + std::rand() % (ROWS - 2))
            };

            // Skip if cell is not free
            if (!isCellFree(cell)) continue;

            // Skip if cell is near starting position
            sf::Vector2u startCell(COLUMNS / 2, ROWS / 2);
            if (std::abs(static_cast<int>(cell.x) - static_cast<int>(startCell.x)) < 5 &&
                std::abs(static_cast<int>(cell.y) - static_cast<int>(startCell.y)) < 5)
            {
                continue;
            }

            obstacles.push_back(cell);
        }
        };

    auto startGame = [&]() {
        snake.clear();
        snake.push_back({ WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f });
        dir = None;
        level = startingLevel;
        lives = INITIAL_LIVES;
        score = 0;
        moveDelay = 0.20f * std::pow(0.9f, level - 1);
        nextLevelScore = 100 * level;
        bonusActive = false;
        bonusSpawnClock.restart();
        spawnObstacles(level);
        spawnFood(); // Properly place initial food
        moveClock.restart();
        };

    GameState state = MainMenu;

    // 7) Main loop
    while (window.isOpen()) {
        // ─── Event handling (SFML 3) ─────────────────────────────────────────
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (state == MainMenu && event->is<sf::Event::MouseButtonPressed>()) {
                auto& mpe = *event->getIf<sf::Event::MouseButtonPressed>();
                if (mpe.button == sf::Mouse::Button::Left) {
                    auto mpos = sf::Mouse::getPosition(window);
                    if (playButton.contains(mpos)) {
                        state = LevelSelect;
                    }
                    else if (exitButton.contains(mpos)) {
                        window.close();
                    }
                }
            }
            else if (state == LevelSelect && event->is<sf::Event::MouseButtonPressed>()) {
                auto& mpe = *event->getIf<sf::Event::MouseButtonPressed>();
                if (mpe.button == sf::Mouse::Button::Left) {
                    auto mpos = sf::Mouse::getPosition(window);
                    for (int i = 0; i < 5; ++i) {
                        if (levelButtons[i].contains(mpos)) {
                            startingLevel = i + 1;
                            startGame();
                            state = Playing;
                            break;
                        }
                    }
                    if (backButton.contains(mpos)) {
                        state = MainMenu;
                    }
                }
            }
            else if (state == GameOver && event->is<sf::Event::MouseButtonPressed>()) {
                auto& mpe = *event->getIf<sf::Event::MouseButtonPressed>();
                if (mpe.button == sf::Mouse::Button::Left) {
                    auto mpos = sf::Mouse::getPosition(window);
                    if (retryButton.contains(mpos)) {
                        startGame();
                        state = Playing;
                    }
                    else if (menuButton.contains(mpos)) {
                        state = MainMenu;
                    }
                    else if (exitButtonGameOver.contains(mpos)) {
                        window.close();
                    }
                }
            }

            if ((state == Playing || state == Paused) && event->is<sf::Event::KeyPressed>()) {
                auto& kpe = *event->getIf<sf::Event::KeyPressed>();
                switch (kpe.scancode) {
                case sf::Keyboard::Scancode::W:
                case sf::Keyboard::Scancode::Up:    if (dir != Down)  dir = Up;    break;
                case sf::Keyboard::Scancode::S:
                case sf::Keyboard::Scancode::Down:  if (dir != Up)    dir = Down;  break;
                case sf::Keyboard::Scancode::A:
                case sf::Keyboard::Scancode::Left:  if (dir != Right) dir = Left;  break;
                case sf::Keyboard::Scancode::D:
                case sf::Keyboard::Scancode::Right: if (dir != Left)  dir = Right; break;
                case sf::Keyboard::Scancode::P:
                    if (state == Playing) { state = Paused; }
                    else { state = Playing; }
                    break;
                case sf::Keyboard::Scancode::M:
                    if (state == Paused) {
                        if (score > highScore) highScore = score;
                        state = MainMenu;
                    }
                    break;
                default: break;
                }
            }
        } // ── end event handling ─────────────────────────────────────────────

        // ─── MainMenu ─────────────────────────────────────────────────────────
        if (state == MainMenu) {
            window.clear(sf::Color::Green);
            highScoreText.setString("High Score: " + std::to_string(highScore));
            centerText(highScoreText, WINDOW_WIDTH / 2.f, 160.f);
            window.draw(titleText);
            window.draw(highScoreText);
            window.draw(playButton.box);
            window.draw(playButton.label);
            window.draw(exitButton.box);
            window.draw(exitButton.label);
            window.display();
            continue;
        }

        // ─── LevelSelect ─────────────────────────────────────────────────────
        if (state == LevelSelect) {
            window.clear(sf::Color::Green);
            sf::Text ls(font, "Select Level", 40);
            ls.setFillColor(sf::Color::Black);
            centerText(ls, WINDOW_WIDTH / 2.f, 100.f);
            window.draw(ls);
            for (auto& b : levelButtons) {
                window.draw(b.box);
                window.draw(b.label);
            }
            window.draw(backButton.box);
            window.draw(backButton.label);
            window.display();
            continue;
        }

        // ─── GameOver ────────────────────────────────────────────────────────
        if (state == GameOver) {
            window.clear(sf::Color(0, 100, 0)); // Dark green background

            finalScoreText.setString("Score: " + std::to_string(score));
            gameOverHighScoreText.setString("High Score: " + std::to_string(highScore));
            centerText(finalScoreText, WINDOW_WIDTH / 2.f, 180.f);
            centerText(gameOverHighScoreText, WINDOW_WIDTH / 2.f, 220.f);

            window.draw(gameOverTitle);
            window.draw(finalScoreText);
            window.draw(gameOverHighScoreText);
            window.draw(retryButton.box);
            window.draw(retryButton.label);
            window.draw(menuButton.box);
            window.draw(menuButton.label);
            window.draw(exitButtonGameOver.box);
            window.draw(exitButtonGameOver.label);

            window.display();
            continue;
        }

        // ─── Playing ─────────────────────────────────────────────────────────
        if (state == Playing) {
            // ── Movement & collision ──────────────────────────────────────
            if (dir != None && moveClock.getElapsedTime().asSeconds() > moveDelay) {
                moveClock.restart();

                // 1) Advance head
                sf::Vector2f head = snake.front();
                switch (dir) {
                case Up:    head.y -= BLOCK_SIZE; break;
                case Down:  head.y += BLOCK_SIZE; break;
                case Left:  head.x -= BLOCK_SIZE; break;
                case Right: head.x += BLOCK_SIZE; break;
                default:    break;
                }

                // 2) Convert to grid cell
                sf::Vector2u cell{
                    static_cast<unsigned>(head.x / BLOCK_SIZE),
                    static_cast<unsigned>(head.y / BLOCK_SIZE)
                };

                // 3) Border collision check
                bool borderCollision = false;
                if (cell.x == 0 || cell.x == COLUMNS - 1 ||
                    cell.y == 0 || cell.y == ROWS - 1) {
                    borderCollision = true;
                }

                // 4) Eat food?
                if (cell == foodCell) {
                    eatSound.play();
                    score += 10;
                    spawnFood(); // Respawn food in valid location
                }
                else {
                    snake.pop_back();
                }

                // 5) Bonus spawn / expire / eat
                if (!bonusActive &&
                    bonusSpawnClock.getElapsedTime().asSeconds() > BONUS_SPAWN_INTERVAL)
                {
                    // Find valid bonus location
                    std::vector<sf::Vector2u> freeCells;
                    for (unsigned y = 1; y < ROWS - 1; ++y) {
                        for (unsigned x = 1; x < COLUMNS - 1; ++x) {
                            sf::Vector2u candidate(x, y);
                            if (isCellFree(candidate) && candidate != foodCell) {
                                freeCells.push_back(candidate);
                            }
                        }
                    }

                    if (!freeCells.empty()) {
                        bonusActive = true;
                        bonusCell = freeCells[std::rand() % freeCells.size()];
                        bonusLiveClock.restart();
                    }
                }
                if (bonusActive &&
                    bonusLiveClock.getElapsedTime().asSeconds() > BONUS_DURATION)
                {
                    bonusActive = false;
                    bonusSpawnClock.restart();
                }
                if (bonusActive && cell == bonusCell) {
                    eatSound.play(); // Same sound as regular food
                    score += 50;
                    bonusActive = false;
                    bonusSpawnClock.restart();
                }

                // 6) Level progression
                if (score >= nextLevelScore) {
                    level++;
                    nextLevelScore += 100;
                    moveDelay *= 0.9f;
                    spawnObstacles(level);

                    // Ensure food is still in valid location
                    if (!isCellFree(foodCell)) {
                        spawnFood();
                    }
                }

                // 7) Insert new head only if no border collision
                if (!borderCollision) {
                    snake.insert(snake.begin(), head);
                }

                // 8) Collision with obstacle or self?
                bool died = false;
                if (borderCollision) {
                    died = true;
                }
                else {
                    for (auto& o : obstacles) {
                        if (cell == o) { died = true; break; }
                    }
                    if (!died) {
                        for (size_t i = 1; i < snake.size(); ++i) {
                            if (snake[i] == snake[0]) { died = true; break; }
                        }
                    }
                }

                if (died) {
                    lives--;
                    if (lives > 0) {
                        // Reset snake position
                        snake.clear();
                        snake.push_back({ WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f });
                        dir = None;
                    }
                    else {
                        gameOverSound.play();
                        if (score > highScore) highScore = score;
                        state = GameOver;
                    }
                }
            }

            // ── Drawing ───────────────────────────────────────────────────
            window.clear(sf::Color::White);

            // Checkerboard
            for (unsigned r = 0; r < ROWS; ++r) {
                for (unsigned c = 0; c < COLUMNS; ++c) {
                    sf::RectangleShape cell;
                    cell.setSize({ BLOCK_SIZE, BLOCK_SIZE });
                    cell.setPosition({ c * BLOCK_SIZE, r * BLOCK_SIZE });
                    cell.setFillColor(((r + c) % 2 == 0) ? BG_COLOR1 : BG_COLOR2);
                    window.draw(cell);
                }
            }

            // Food
            {
                sf::CircleShape food(BLOCK_SIZE / 2.f);
                food.setFillColor(sf::Color::White);
                food.setOrigin({ BLOCK_SIZE / 2.f, BLOCK_SIZE / 2.f });
                food.setPosition({
                    foodCell.x * BLOCK_SIZE + BLOCK_SIZE / 2.f,
                    foodCell.y * BLOCK_SIZE + BLOCK_SIZE / 2.f
                    });
                window.draw(food);
            }

            // Bonus
            if (bonusActive) {
                sf::CircleShape bonus(BLOCK_SIZE / 2.f);
                bonus.setFillColor(BONUS_COLOR);
                bonus.setOrigin({ BLOCK_SIZE / 2.f, BLOCK_SIZE / 2.f });
                bonus.setPosition({
                    bonusCell.x * BLOCK_SIZE + BLOCK_SIZE / 2.f,
                    bonusCell.y * BLOCK_SIZE + BLOCK_SIZE / 2.f
                    });
                window.draw(bonus);
            }

            // Obstacles
            for (auto& o : obstacles) {
                sf::RectangleShape obs;
                obs.setSize({ BLOCK_SIZE - 2.f, BLOCK_SIZE - 2.f });
                obs.setFillColor(OBSTACLE_COLOR);
                obs.setPosition({ o.x * BLOCK_SIZE + 1.f, o.y * BLOCK_SIZE + 1.f });
                window.draw(obs);
            }

            // Snake
            for (size_t i = 0; i < snake.size(); ++i) {
                sf::CircleShape part(BLOCK_SIZE / 2.f);
                part.setOrigin({ BLOCK_SIZE / 2.f, BLOCK_SIZE / 2.f });
                part.setPosition({ snake[i].x + BLOCK_SIZE / 2.f,
                                   snake[i].y + BLOCK_SIZE / 2.f });
                if (i == 0) {
                    int tint = (level * 5) % 256;
                    part.setFillColor({ static_cast<uint8_t>((255 + tint) % 256), 0, 255 });
                }
                else {
                    part.setFillColor({ 128, 0, 128 });
                }
                window.draw(part);
            }

            // Borders
            window.draw(borderTop);
            window.draw(borderBottom);
            window.draw(borderLeft);
            window.draw(borderRight);

            // Info text
            infoText.setString(
                "Lives: " + std::to_string(lives) +
                "    Score: " + std::to_string(score) +
                "    Level: " + std::to_string(level)
            );
            window.draw(infoText);

            window.display();
        }
        // ─── Paused ─────────────────────────────────────────────────────────
        else if (state == Paused) {
            window.clear(sf::Color(0, 0, 0, 150));
            window.draw(pauseText);
            window.display();
        }
    }

    return 0;
}