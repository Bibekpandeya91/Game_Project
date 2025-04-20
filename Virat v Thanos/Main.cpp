// SFML Maze Game - Combined Single File Version (Corrected Order)

// ==========================================================================
// 1. Includes
// ==========================================================================
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <string>
#include <memory> // For unique_ptr
#include <iostream>
#include <fstream>
#include <cstdlib> // For rand(), srand()
#include <ctime>   // For time()
#include <algorithm> // For std::remove_if
#include <stdexcept> // For std::exception in main

// ==========================================================================
// 2. Using Namespaces
// ==========================================================================
using namespace sf;
using namespace std;

// ==========================================================================
// 3. Global Definitions
// ==========================================================================
const int GRID_WIDTH = 10;
const int GRID_HEIGHT = 10;
const float CELL_SIZE = 60.f;
const float WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
const float WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE + 100;
const char WALL_CHAR = '#';
const char PATH_CHAR = ' ';
const char PLAYER_CHAR = 'P';
const char ITEM_CHAR = '*';
const char ENEMY_CHAR = 'X';
const float ENEMY_MOVE_INTERVAL = 1.0f;
const float SHOOT_COOLDOWN = 0.3f;
const float BULLET_TIME_PER_STEP = 0.05f;
const string PLAYER_TEXTURE_PATH = "C:/Users/bibek/source/repos/MYTRY/x64/Debug/assets/player.png"; // !! ABSOLUTE PATH !!
const string ENEMY_TEXTURE_PATH = "assets/enemy.png"; // Relative path

// ==========================================================================
// 4. Forward Declarations
// ==========================================================================
class Level;
class Game;
class Entity;
class Player;
class Enemy;
class Bullet;
enum class GameState; // Defined later

// ==========================================================================
// 5. Entity Class Definition
// ==========================================================================
class Entity {
public:
    Entity(int startX, int startY);
    virtual ~Entity() = default;
    virtual void setPosition(int x, int y);
    Vector2i getPosition() const;
    bool isActive() const;
    virtual void destroy();
    virtual void update(float dt, const Level& level, vector<Entity*>& others, Game& game) = 0;
    virtual void draw(RenderWindow& window, float cellSize) const = 0;
protected:
    Vector2i position;
    bool active;
};

// ==========================================================================
// 6. Level Class Definition
// ==========================================================================
class Level {
public:
    Level();
    bool loadFromFile(const string& filename);
    bool saveToFile(const string& filename) const;
    void draw(RenderWindow& window, float cellSize);
    char getCell(int x, int y) const;
    void setCell(int x, int y, char type);
    size_t getWidth() const;
    size_t getHeight() const;
    Vector2i findChar(char target) const;
    bool isWall(int x, int y) const;
    bool isPath(int x, int y) const;
    bool isItem(int x, int y) const;
    bool isEnemySpawn(int x, int y) const;
    bool isValid(int x, int y) const;
private:
    vector<string> grid;
    size_t width;
    size_t height;
    RectangleShape cellShape;
    CircleShape itemShape;
};

// ==========================================================================
// 7. Bullet Class Definition
// ==========================================================================
class Bullet : public Entity {
public:
    Bullet(int startX, int startY, int dirX, int dirY);
    void update(float dt, const Level& level, vector<Entity*>& others, Game& game) override;
    void draw(RenderWindow& window, float cellSize) const override;
    Vector2i getVelocity() const;
private:
    Vector2i velocity;
    RectangleShape shape;
    float moveTimer;
    const float timePerStep = BULLET_TIME_PER_STEP;
};

// ==========================================================================
// 8. Player Class Definition
// ==========================================================================
class Player : public Entity {
public:
    Player(int startX, int startY, const Texture& texture);
    void handleInput(Keyboard::Key key, Level& level, vector<unique_ptr<Bullet>>& bullets, Game& game);
    void update(float dt, const Level& level, vector<Entity*>& others, Game& game) override;
    void draw(RenderWindow& window, float cellSize) const override;
    void addScore(int points);
    int getScore() const;
    void reset();
    void setPosition(int x, int y) override;
private:
    int score;
    Sprite sprite;
    Vector2i facingDirection;
    Clock shootTimer;
    void tryMove(int dx, int dy, Level& level, Game& game); // Uses Game& game
    void shoot(vector<unique_ptr<Bullet>>& bullets, const Level& level);
    void updateSpritePosition(float cellSize);
};

// ==========================================================================
// 9. Enemy Class Definition
// ==========================================================================
class Enemy : public Entity {
public:
    Enemy(int startX, int startY, const Texture& texture);
    void update(float dt, const Level& level, vector<Entity*>& others, Game& game) override;
    void draw(RenderWindow& window, float cellSize) const override;
private:
    Sprite sprite;
    Clock moveTimer;
    bool tryMoveRandom(const Level& level);
    void updateSpritePosition(float cellSize);
};

// ==========================================================================
// 10. Game State Enum Definition
// ==========================================================================
enum class GameState { Playing, Paused, GameOver, Victory, LevelComplete };

// ==========================================================================
// 11. Game Class Definition
// ==========================================================================
class Game {
public:
    Game();
    ~Game() = default;
    void run();
    void setGameOver(const string& message); // Method used by Player
private:
    void processEvents();
    void update(float dt);
    void render();
    void loadLevel(int levelNumber);
    void setupLevel();
    void nextLevel();
    void resetGame();
    void setupUI();
    void updateUI();
    void checkCollisions();
    void cleanupEntities();
    bool loadTextures();

    RenderWindow window;
    Texture playerTexture;
    Texture enemyTexture;
    Font font;
    Level currentLevelData;
    unique_ptr<Player> player_ptr;
    vector<Enemy> enemies;
    vector<unique_ptr<Bullet>> bullets;
    GameState currentState;
    int currentLevelIndex;
    int totalLevels;
    float timeScale;
    Text scoreText;
    Text levelText;
    Text messageText;
};

// ==========================================================================
// ==========================================================================
// Implementations START here, AFTER all class definitions
// ==========================================================================
// ==========================================================================


// ==========================================================================
// Entity Implementation
// ==========================================================================
Entity::Entity(int startX, int startY) : position(startX, startY), active(true) {}

void Entity::setPosition(int x, int y) {
    position.x = x;
    position.y = y;
}

Vector2i Entity::getPosition() const { return position; }
bool Entity::isActive() const { return active; }
void Entity::destroy() { active = false; }
// update, draw are pure virtual

// ==========================================================================
// Level Implementation
// ==========================================================================
Level::Level() : width(0), height(0) {
    cellShape.setOutlineThickness(1.f);
    cellShape.setOutlineColor(Color(50, 50, 50));
    itemShape.setRadius(CELL_SIZE * 0.2f);
    itemShape.setFillColor(Color::Magenta);
    itemShape.setOrigin(itemShape.getRadius(), itemShape.getRadius());
}

bool Level::loadFromFile(const string& filename) {
    ifstream inputFile(filename);
    if (!inputFile) {
        cerr << "Error: Could not open level file: " << filename << endl; return false;
    }
    grid.clear();
    string line;
    size_t tempWidth = 0;
    bool firstLine = true;
    while (getline(inputFile, line)) {
        if (line.empty()) continue;
        if (firstLine) {
            tempWidth = line.length(); firstLine = false;
        }
        else if (line.length() != tempWidth) {
            cerr << "Error: Inconsistent line length in level file: " << filename << endl;
            inputFile.close(); return false;
        }
        grid.push_back(line);
    }
    inputFile.close();
    if (grid.empty()) {
        cerr << "Error: Level file is empty: " << filename << endl;
        height = 0; width = 0; return false;
    }
    height = grid.size();
    width = tempWidth;
    cout << "Loaded level '" << filename << "' (" << width << "x" << height << ")" << endl;
    return true;
}

bool Level::saveToFile(const string& filename) const {
    ofstream outputFile(filename);
    if (!outputFile) {
        cerr << "Error: Could not open file for saving level: " << filename << endl; return false;
    }
    for (const auto& row : grid) { outputFile << row << endl; }
    outputFile.close();
    cout << "Saved level layout to '" << filename << "'" << endl;
    return true;
}

void Level::draw(RenderWindow& window, float cellSize) {
    cellShape.setSize(Vector2f(cellSize, cellSize));
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            cellShape.setPosition(static_cast<float>(x) * cellSize, static_cast<float>(y) * cellSize);
            char cellType = grid[y][x];
            Color fillColor = (cellType == WALL_CHAR) ? Color(100, 100, 255) : Color(40, 40, 40);
            cellShape.setFillColor(fillColor);
            window.draw(cellShape);
            if (cellType == ITEM_CHAR) {
                itemShape.setPosition(static_cast<float>(x) * cellSize + cellSize / 2.f, static_cast<float>(y) * cellSize + cellSize / 2.f);
                window.draw(itemShape);
            }
        }
    }
}

char Level::getCell(int x, int y) const {
    if (!isValid(x, y)) { return WALL_CHAR; }
    return grid[static_cast<size_t>(y)][static_cast<size_t>(x)];
}

void Level::setCell(int x, int y, char type) {
    if (isValid(x, y)) {
        grid[static_cast<size_t>(y)][static_cast<size_t>(x)] = type;
    }
    else {
        cerr << "Warning: Attempted to set cell outside level bounds (" << x << "," << y << ")" << endl;
    }
}

size_t Level::getWidth() const { return width; }
size_t Level::getHeight() const { return height; }

Vector2i Level::findChar(char target) const {
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            if (grid[y][x] == target) { return Vector2i(static_cast<int>(x), static_cast<int>(y)); }
        }
    }
    return Vector2i(-1, -1);
}

bool Level::isValid(int x, int y) const {
    return x >= 0 && static_cast<size_t>(x) < width && y >= 0 && static_cast<size_t>(y) < height;
}

bool Level::isWall(int x, int y) const {
    if (!isValid(x, y)) { return true; }
    return grid[static_cast<size_t>(y)][static_cast<size_t>(x)] == WALL_CHAR;
}

bool Level::isPath(int x, int y) const {
    if (!isValid(x, y)) { return false; }
    char cell = grid[static_cast<size_t>(y)][static_cast<size_t>(x)];
    return cell == PATH_CHAR || cell == ITEM_CHAR || cell == PLAYER_CHAR || cell == ENEMY_CHAR;
}

bool Level::isItem(int x, int y) const {
    if (!isValid(x, y)) { return false; }
    return grid[static_cast<size_t>(y)][static_cast<size_t>(x)] == ITEM_CHAR;
}

bool Level::isEnemySpawn(int x, int y) const {
    if (!isValid(x, y)) { return false; }
    return grid[static_cast<size_t>(y)][static_cast<size_t>(x)] == ENEMY_CHAR;
}


// ==========================================================================
// Bullet Implementation
// ==========================================================================
Bullet::Bullet(int startX, int startY, int dirX, int dirY)
    : Entity(startX, startY), velocity(dirX, dirY), moveTimer(0.0f) {
    shape.setSize(Vector2f(CELL_SIZE * 0.2f, CELL_SIZE * 0.2f));
    shape.setFillColor(Color::Yellow);
    shape.setOrigin(shape.getSize().x / 2.f, shape.getSize().y / 2.f);
    shape.setPosition(position.x * CELL_SIZE + CELL_SIZE / 2.f, position.y * CELL_SIZE + CELL_SIZE / 2.f);
}

// NOTE: This update currently does NOT call any Game methods, so its position
// relative to Game definition isn't strictly critical for *this* version.
// However, placing it after Game definition is safer if it might need Game in the future.
void Bullet::update(float dt, const Level& level, vector<Entity*>& others, Game& game) {
    if (!active) return;
    moveTimer += dt;
    bool moved = false;
    while (moveTimer >= timePerStep && active) {
        moveTimer -= timePerStep;
        int nextX = position.x + velocity.x;
        int nextY = position.y + velocity.y;
        if (!level.isValid(nextX, nextY) || level.isWall(nextX, nextY)) {
            destroy();
            return;
        }
        position.x = nextX;
        position.y = nextY;
        moved = true;
        // Collision with enemies handled in Game::checkCollisions
    }
    if (moved) {
        shape.setPosition(position.x * CELL_SIZE + CELL_SIZE / 2.f, position.y * CELL_SIZE + CELL_SIZE / 2.f);
    }
}

void Bullet::draw(RenderWindow& window, float cellSize) const {
    if (!active) return;
    window.draw(shape);
}

Vector2i Bullet::getVelocity() const { return velocity; }

// ==========================================================================
// Player Implementation
// ==========================================================================
Player::Player(int startX, int startY, const Texture& texture)
    : Entity(startX, startY), score(0), facingDirection(0, -1) {
    sprite.setTexture(texture);
    sprite.setOrigin(sprite.getLocalBounds().width / 2.f, sprite.getLocalBounds().height / 2.f);
    float desiredWidth = CELL_SIZE * 0.8f;
    float scale = (sprite.getLocalBounds().width > 0) ? desiredWidth / sprite.getLocalBounds().width : 1.0f;
    if (scale == 1.0f && sprite.getLocalBounds().width <= 0) { cerr << "Warning: Player texture width zero." << endl; }
    sprite.setScale(scale, scale);
    shootTimer.restart();
    updateSpritePosition(CELL_SIZE);
}

void Player::handleInput(Keyboard::Key key, Level& level, vector<unique_ptr<Bullet>>& bullets, Game& game) {
    if (!active) return;
    int dx = 0, dy = 0;
    switch (key) {
    case Keyboard::W: dy = -1; break;
    case Keyboard::S: dy = 1;  break;
    case Keyboard::A: dx = -1; break;
    case Keyboard::D: dx = 1;  break;
    case Keyboard::Space:
        if (shootTimer.getElapsedTime().asSeconds() >= SHOOT_COOLDOWN) {
            shoot(bullets, level);
            shootTimer.restart();
        }
        return;
    default: return;
    }
    if (dx != 0 || dy != 0) {
        facingDirection = { dx, dy };
        tryMove(dx, dy, level, game); // Calls method below
    }
}

// THIS METHOD MUST BE IMPLEMENTED *AFTER* THE Game CLASS DEFINITION
void Player::tryMove(int dx, int dy, Level& level, Game& game) {
    if (!active) return;
    int nextX = position.x + dx;
    int nextY = position.y + dy;

    if (level.isWall(nextX, nextY)) {
        cout << "Player hit wall!" << endl;
        destroy();
        game.setGameOver("You walked into a wall!"); // Needs full Game definition
        return;
    }
    if (!level.isValid(nextX, nextY)) {
        cout << "Player hit boundary!" << endl;
        destroy();
        game.setGameOver("You fell off the edge!"); // Needs full Game definition
        return;
    }

    position.x = nextX;
    position.y = nextY;
    if (level.isItem(nextX, nextY)) {
        addScore(10);
        level.setCell(nextX, nextY, PATH_CHAR);
        cout << "Collected item! Score: " << score << endl;
    }
    updateSpritePosition(CELL_SIZE);
}

void Player::shoot(vector<unique_ptr<Bullet>>& bullets, const Level& level) {
    int bulletStartX = position.x + facingDirection.x;
    int bulletStartY = position.y + facingDirection.y;
    if (level.isValid(bulletStartX, bulletStartY) && !level.isWall(bulletStartX, bulletStartY)) {
        bullets.push_back(make_unique<Bullet>(bulletStartX, bulletStartY, facingDirection.x, facingDirection.y));
    }
    else {
        cout << "Blocked shot." << endl;
    }
}

void Player::update(float dt, const Level& level, vector<Entity*>& others, Game& game) {
    if (!active) return;
    // Passive effects can go here
}

void Player::draw(RenderWindow& window, float cellSize) const {
    if (!active) return;
    window.draw(sprite);
}

void Player::addScore(int points) { score += points; }
int Player::getScore() const { return score; }

void Player::reset() {
    score = 0;
    active = true;
    facingDirection = { 0, -1 };
    shootTimer.restart();
    // Position reset by Game::setupLevel via Player::setPosition
}

void Player::setPosition(int x, int y) {
    Entity::setPosition(x, y);
    updateSpritePosition(CELL_SIZE);
}

void Player::updateSpritePosition(float cellSize) {
    sprite.setPosition(position.x * cellSize + cellSize / 2.f, position.y * cellSize + cellSize / 2.f);
}

// ==========================================================================
// Enemy Implementation
// ==========================================================================
Enemy::Enemy(int startX, int startY, const Texture& texture) : Entity(startX, startY) {
    sprite.setTexture(texture);
    sprite.setOrigin(sprite.getLocalBounds().width / 2.f, sprite.getLocalBounds().height / 2.f);
    float desiredWidth = CELL_SIZE * 0.8f;
    float scale = (sprite.getLocalBounds().width > 0) ? desiredWidth / sprite.getLocalBounds().width : 1.0f;
    if (scale == 1.0f && sprite.getLocalBounds().width <= 0) { cerr << "Warning: Enemy texture width zero." << endl; }
    sprite.setScale(scale, scale);
    updateSpritePosition(CELL_SIZE);
    moveTimer.restart();
}

// NOTE: This update currently does NOT call any Game methods.
void Enemy::update(float dt, const Level& level, vector<Entity*>& others, Game& game) {
    if (!active) return;
    bool moved = false;
    if (moveTimer.getElapsedTime().asSeconds() >= ENEMY_MOVE_INTERVAL) {
        moved = tryMoveRandom(level);
        moveTimer.restart();
    }
    if (moved) {
        updateSpritePosition(CELL_SIZE);
    }
}

bool Enemy::tryMoveRandom(const Level& level) {
    if (!active) return false;
    int dx = 0, dy = 0;
    int direction = rand() % 5;
    switch (direction) {
    case 0: dy = -1; break; case 1: dy = 1; break;
    case 2: dx = -1; break; case 3: dx = 1; break;
    case 4: return false;
    }
    int nextX = position.x + dx;
    int nextY = position.y + dy;
    if (level.isValid(nextX, nextY) && !level.isWall(nextX, nextY) && !level.isItem(nextX, nextY)) {
        position.x = nextX;
        position.y = nextY;
        return true;
    }
    return false;
}

void Enemy::draw(RenderWindow& window, float cellSize) const {
    if (!active) return;
    window.draw(sprite);
}

void Enemy::updateSpritePosition(float cellSize) {
    sprite.setPosition(position.x * cellSize + cellSize / 2.f, position.y * cellSize + cellSize / 2.f);
}

// ==========================================================================
// Game Implementation
// ==========================================================================
Game::Game() :
    window(VideoMode(static_cast<unsigned int>(WINDOW_WIDTH), static_cast<unsigned int>(WINDOW_HEIGHT)), "Virat v thanos"),
    currentState(GameState::Playing), currentLevelIndex(1), totalLevels(2), timeScale(1.0f) {
    window.setFramerateLimit(60);
    srand(static_cast<unsigned int>(time(NULL)));
    cout << "Game Constructor: Initializing..." << endl;
    if (!loadTextures()) {
        cerr << "FATAL ERROR: Texture loading failed. Check paths/files.\n";
        currentState = GameState::GameOver; window.close();
        messageText.setString("FATAL ERROR:\nTextures missing."); // Basic msg
        return;
    }
    player_ptr = make_unique<Player>(0, 0, playerTexture);
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Error: Font 'arial.ttf' not found.\n";
        // Continue without text? Or make fatal? For now, continue.
    }
    else {
        cout << "Font loaded." << endl;
    }
    setupUI();
    loadLevel(currentLevelIndex); // Includes setupLevel()
    cout << "Game Constructor: Done." << endl;
}

bool Game::loadTextures() {
    bool pOk = playerTexture.loadFromFile(PLAYER_TEXTURE_PATH);
    if (pOk) playerTexture.setSmooth(true); else cerr << "Failed to load: " << PLAYER_TEXTURE_PATH << endl;
    bool eOk = enemyTexture.loadFromFile(ENEMY_TEXTURE_PATH);
    if (eOk) enemyTexture.setSmooth(true); else cerr << "Failed to load: " << ENEMY_TEXTURE_PATH << endl;
    return pOk && eOk;
}

void Game::run() {
    if (!window.isOpen()) {
        cerr << "Window failed to open or closed during init. Exiting." << endl;
        // Simple error display if possible
        RenderWindow errorWin(VideoMode(400, 100), "Init Error");
        Text errorTxt("Initialization Failed.\nCheck Console/Logs.", font, 20);
        errorTxt.setFillColor(Color::Red);
        errorWin.clear(); errorWin.draw(errorTxt); errorWin.display(); sleep(seconds(5));
        return;
    }
    cout << "Starting Game Loop..." << endl;
    Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds() * timeScale;
        processEvents();
        if (currentState == GameState::Playing) { update(dt); }
        render();
    }
    cout << "Exited Game Loop." << endl;
}

void Game::processEvents() {
    Event event;
    while (window.pollEvent(event)) {
        if (event.type == Event::Closed) { window.close(); }
        if (event.type == Event::KeyPressed) {
            if (event.key.code == Keyboard::P) {
                if (currentState == GameState::Playing) { currentState = GameState::Paused; timeScale = 0.0f; messageText.setString("PAUSED\nPress P"); }
                else if (currentState == GameState::Paused) { currentState = GameState::Playing; timeScale = 1.0f; messageText.setString(""); }
            }
            else if ((currentState == GameState::GameOver || currentState == GameState::Victory) && event.key.code == Keyboard::R) { resetGame(); }
            else if (currentState == GameState::Playing && player_ptr && player_ptr->isActive()) {
                player_ptr->handleInput(event.key.code, currentLevelData, bullets, *this);
            }
        }
    }
}

void Game::update(float dt) {
    if (currentState != GameState::Playing || !player_ptr || !player_ptr->isActive()) return;
    vector<Entity*> others_placeholder;
    player_ptr->update(dt, currentLevelData, others_placeholder, *this);
    if (currentState != GameState::Playing) return; // State might change in player update
    for (auto& enemy : enemies) { if (enemy.isActive()) enemy.update(dt, currentLevelData, others_placeholder, *this); }
    for (auto& bullet_ptr : bullets) { if (bullet_ptr->isActive()) bullet_ptr->update(dt, currentLevelData, others_placeholder, *this); }
    checkCollisions();
    if (currentState != GameState::Playing) return; // State might change in collisions
    cleanupEntities();
    bool enemiesRemaining = any_of(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.isActive(); });
    if (!enemiesRemaining && currentState == GameState::Playing) {
        if (currentLevelIndex < totalLevels) { nextLevel(); }
        else {
            currentState = GameState::Victory;
            string scoreStr = player_ptr ? to_string(player_ptr->getScore()) : "N/A";
            messageText.setString("YOU WIN!\nScore: " + scoreStr + "\nPress R");
            if (player_ptr && player_ptr->isActive()) player_ptr->destroy();
        }
    }
    if (currentState == GameState::Playing || currentState == GameState::Paused) { updateUI(); }
}

void Game::checkCollisions() {
    if (currentState != GameState::Playing || !player_ptr || !player_ptr->isActive()) return;
    Vector2i pPos = player_ptr->getPosition();
    // Player vs Enemies
    for (auto& enemy : enemies) {
        if (enemy.isActive() && enemy.getPosition() == pPos) {
            setGameOver("Caught by an enemy!"); return;
        }
    }
    // Bullets vs Enemies
    for (auto& bullet_ptr : bullets) {
        if (!bullet_ptr->isActive()) continue;
        Vector2i bPos = bullet_ptr->getPosition();
        for (auto& enemy : enemies) {
            if (enemy.isActive() && enemy.getPosition() == bPos) {
                cout << "Hit! Enemy destroyed." << endl;
                enemy.destroy();
                bullet_ptr->destroy();
                if (player_ptr) player_ptr->addScore(50);
                break; // Bullet hits one enemy max
            }
        }
        if (!bullet_ptr->isActive()) continue; // If bullet was destroyed, move to next bullet
    }
}

void Game::cleanupEntities() {
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const unique_ptr<Bullet>& b) { return !b->isActive(); }), bullets.end());
    enemies.erase(remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.isActive(); }), enemies.end());
}

void Game::render() {
    window.clear(Color(20, 20, 20));
    currentLevelData.draw(window, CELL_SIZE);
    for (const auto& enemy : enemies) { if (enemy.isActive()) enemy.draw(window, CELL_SIZE); }
    for (const auto& bullet_ptr : bullets) { if (bullet_ptr->isActive()) bullet_ptr->draw(window, CELL_SIZE); }
    if (player_ptr && player_ptr->isActive()) { player_ptr->draw(window, CELL_SIZE); }
    window.draw(scoreText);
    window.draw(levelText);
    if (currentState != GameState::Playing && currentState != GameState::LevelComplete) {
        RectangleShape overlay(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(Color(0, 0, 0, 180));
        window.draw(overlay);
        FloatRect textRect = messageText.getLocalBounds();
        messageText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        messageText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
        window.draw(messageText);
    }
    window.display();
}

void Game::loadLevel(int levelNumber) {
    cout << "Loading level " << levelNumber << "..." << endl;
    string filename = "level" + to_string(levelNumber) + ".txt";
    if (!currentLevelData.loadFromFile(filename)) {
        cerr << "Error loading " << filename << endl;
        if (levelNumber != 1) { cout << "Falling back to level 1." << endl; loadLevel(1); }
        else { setGameOver("FATAL: Cannot load level1.txt!"); }
        return;
    }
    currentLevelIndex = levelNumber;
    if (player_ptr) {
        setupLevel();
        currentState = GameState::Playing; timeScale = 1.0f; messageText.setString("");
    }
    else {
        setGameOver("FATAL: Player null during loadLevel!");
    }
}

void Game::setupLevel() {
    cout << "Setting up level " << currentLevelIndex << "..." << endl;
    bullets.clear(); enemies.clear();
    if (!player_ptr) { currentState = GameState::GameOver; messageText.setString("FATAL:\nPlayer setup fail."); return; }
    Vector2i playerStart = currentLevelData.findChar(PLAYER_CHAR);
    if (playerStart.x == -1) {
        cerr << "Warning: 'P' not found. Defaulting/Searching..." << endl;
        playerStart = { 1, 1 }; // Default
        if (currentLevelData.isWall(playerStart.x, playerStart.y)) { // Search if default is wall
            playerStart = { -1,-1 };
            for (size_t y = 0; y < currentLevelData.getHeight() && playerStart.x == -1; ++y)
                for (size_t x = 0; x < currentLevelData.getWidth() && playerStart.x == -1; ++x)
                    if (!currentLevelData.isWall(static_cast<int>(x), static_cast<int>(y)))
                        playerStart = { static_cast<int>(x), static_cast<int>(y) };
            if (playerStart.x == -1) playerStart = { 0,0 }; // Absolute fallback
            cerr << "Warning: Found valid start at (" << playerStart.x << "," << playerStart.y << ")" << endl;
        }
    }
    player_ptr->reset();
    player_ptr->setPosition(playerStart.x, playerStart.y);
    for (size_t y = 0; y < currentLevelData.getHeight(); ++y) {
        for (size_t x = 0; x < currentLevelData.getWidth(); ++x) {
            if (currentLevelData.getCell(static_cast<int>(x), static_cast<int>(y)) == ENEMY_CHAR) {
                enemies.emplace_back(static_cast<int>(x), static_cast<int>(y), enemyTexture);
            }
        }
    }
    updateUI();
    cout << "Level " << currentLevelIndex << " setup: Player (" << playerStart.x << "," << playerStart.y << "), Enemies: " << enemies.size() << endl;
}

void Game::nextLevel() {
    cout << "Advancing level..." << endl;
    if (currentLevelIndex < totalLevels) {
        currentLevelIndex++; loadLevel(currentLevelIndex);
    }
    else {
        if (currentState != GameState::Victory) { // Prevent multiple calls
            currentState = GameState::Victory;
            string scoreStr = player_ptr ? to_string(player_ptr->getScore()) : "N/A";
            messageText.setString("YOU WIN!\nScore: " + scoreStr + "\nPress R");
            if (player_ptr && player_ptr->isActive()) player_ptr->destroy();
        }
    }
}

void Game::resetGame() {
    cout << "Resetting game..." << endl;
    currentState = GameState::Playing; timeScale = 1.0f; currentLevelIndex = 1; messageText.setString("");
    loadLevel(currentLevelIndex); // This handles setup
}

void Game::setupUI() {
    float uiY = static_cast<float>(GRID_HEIGHT * CELL_SIZE) + 30.f;
    uiY = std::min(uiY, WINDOW_HEIGHT - 50.f); // Clamp Y
    scoreText.setFont(font); scoreText.setCharacterSize(24); scoreText.setFillColor(Color::White); scoreText.setPosition(20.f, uiY);
    levelText.setFont(font); levelText.setCharacterSize(24); levelText.setFillColor(Color::White); levelText.setPosition(WINDOW_WIDTH - 150.f, uiY);
    messageText.setFont(font); messageText.setCharacterSize(40); messageText.setFillColor(Color::Yellow); messageText.setStyle(Text::Bold);
}

void Game::updateUI() {
    scoreText.setString("Score: " + (player_ptr ? to_string(player_ptr->getScore()) : "N/A"));
    levelText.setString("Level: " + to_string(currentLevelIndex));
}

// Must be defined AFTER Game class definition
void Game::setGameOver(const string& message) {
    if (currentState == GameState::Playing) {
        cout << "GAME OVER: " << message << endl;
        currentState = GameState::GameOver;
        timeScale = 0.0f;
        messageText.setString("GAME OVER!\n" + message + "\nPress R to Restart");
        if (player_ptr && player_ptr->isActive()) { player_ptr->destroy(); }
    }
    else {
        cout << "setGameOver called when not playing. State: " << static_cast<int>(currentState) << endl;
    }
}

// ==========================================================================
// Main Function
// ==========================================================================
int main() {
    cout << "Application Start..." << endl;
    try {
        Game game;
        game.run();
    }
    catch (const exception& e) {
        cerr << "Unhandled Exception: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        cerr << "Unknown Unhandled Exception." << endl;
        return EXIT_FAILURE;
    }
    cout << "Application Exit." << endl;
    return EXIT_SUCCESS;
}