#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* backgroundTexture = NULL;
SDL_Texture* snakeHead = NULL;
SDL_Texture* snakeBody = NULL;
SDL_Texture* snakeTail = NULL;
SDL_Texture* foodTexture = NULL;
bool isRunning;
int snakeSegmentSize = 24;  // Size of each snake segment
Uint32 snakeSpeed = 200; // Movement delay in milliseconds
Uint32 lastMoveTime = 0; // Last time the snake moved

enum Direction { UP, DOWN, LEFT, RIGHT };
Direction dir = RIGHT;

struct Segment {
    int x, y;
    SDL_Texture* texture;
    double angle;
};

struct Food {
    int x, y;
    SDL_Texture* texture;
};

vector<Segment> snake;
Food food;

// Function to load an image as a texture
SDL_Texture* loadTexture(const std::string &path, SDL_Renderer *renderer) {
    SDL_Texture *newTexture = NULL;
    SDL_Surface *loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
    } else {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == NULL) {
            std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

void placeFood() {
    bool validPosition;
    do {
        validPosition = true;
        food.x = (rand() % (SCREEN_WIDTH / snakeSegmentSize)) * snakeSegmentSize;
        food.y = (rand() % (SCREEN_HEIGHT / snakeSegmentSize)) * snakeSegmentSize;

        // Ensure the food does not spawn on the snake
        for (const Segment& segment : snake) {
            if (food.x == segment.x && food.y == segment.y) {
                validPosition = false;
                break;
            }
        }
    } while (!validPosition);
}

bool initializeWindow() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cout << "Error: SDL initialization failed\nSDL Error: " << SDL_GetError() << endl;
        isRunning = false;
        return false;
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cout << "Error: Failed to create window\nSDL Error: " << SDL_GetError() << endl;
        isRunning = false;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cout << "Error: Failed to create renderer\nSDL Error: " << SDL_GetError() << endl;
        isRunning = false;
        return false;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cout << "Error: Failed to initialize SDL_image\nSDL_image Error: " << IMG_GetError() << endl;
        isRunning = false;
        return false;
    }

    // Load background texture
    backgroundTexture = loadTexture("resource/g1.png", renderer);
    if (!backgroundTexture) {
        cout << "Error: Failed to load background texture" << endl;
        isRunning = false;
        return false;
    }

    // Load snake head texture
    snakeHead = loadTexture("resource/head.png", renderer);
    if (!snakeHead) {
        cout << "Error: Failed to load snake head texture" << endl;
        isRunning = false;
        return false;
    }

    // Load snake body texture
    snakeBody = loadTexture("resource/body.png", renderer);
    if (!snakeBody) {
        cout << "Error: Failed to load snake body texture" << endl;
        isRunning = false;
        return false;
    }

    // Load snake tail texture
    snakeTail = loadTexture("resource/tail.png", renderer);
    if (!snakeTail) {
        cout << "Error: Failed to load snake tail texture" << endl;
        isRunning = false;
        return false;
    }

    // Load food texture
    foodTexture = loadTexture("resource/food.png", renderer);
    if (!foodTexture) {
        cout << "Error: Failed to load food texture" << endl;
        isRunning = false;
        return false;
    }

    // Initialize snake
    snake.push_back({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, snakeHead, 0.0});
    snake.push_back({SCREEN_WIDTH / 2 - snakeSegmentSize, SCREEN_HEIGHT / 2, snakeBody, 0.0});
    snake.push_back({SCREEN_WIDTH / 2 - 2 * snakeSegmentSize, SCREEN_HEIGHT / 2, snakeTail, 0.0});

    // Initialize food
    food.texture = foodTexture;
    placeFood();

    return true;
}

void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (dir != DOWN) dir = UP;
                    break;
                case SDLK_DOWN:
                    if (dir != UP) dir = DOWN;
                    break;
                case SDLK_LEFT:
                    if (dir != RIGHT) dir = LEFT;
                    break;
                case SDLK_RIGHT:
                    if (dir != LEFT) dir = RIGHT;
                    break;
            }
        }
    }
}

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    if((a.y + a.h) >= b.y && a.y <= (b.y + b.h) && (a.x + a.w) >= b.x && a.x <= (b.x + b.w)) return true;
    return false;
}

void update() {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastMoveTime < snakeSpeed) {
        return;
    }
    lastMoveTime = currentTime;

    // Update snake segments positions
    for (int i = snake.size() - 1; i > 0; --i) {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
        snake[i].angle = snake[i - 1].angle;
    }

    // Update head position
    switch (dir) {
        case UP:
            snake[0].y -= snakeSegmentSize;
            snake[0].angle = 270.0;
            break;
        case DOWN:
            snake[0].y += snakeSegmentSize;
            snake[0].angle = 90.0;
            break;
        case LEFT:
            snake[0].x -= snakeSegmentSize;
            snake[0].angle = 180.0;
            break;
        case RIGHT:
            snake[0].x += snakeSegmentSize;
            snake[0].angle = 0.0;
            break;
    }

    // Check boundaries
    if (snake[0].x < 0) snake[0].x = SCREEN_WIDTH - snakeSegmentSize;
    if (snake[0].x >= SCREEN_WIDTH) snake[0].x = 0;
    if (snake[0].y < 0) snake[0].y = SCREEN_HEIGHT - snakeSegmentSize;
    if (snake[0].y >= SCREEN_HEIGHT) snake[0].y = 0;

    // Check for food collision
    SDL_Rect headRect = {snake[0].x, snake[0].y, snakeSegmentSize, snakeSegmentSize};
    SDL_Rect foodRect = {food.x, food.y, snakeSegmentSize, snakeSegmentSize};
    
    // Debug: Print positions
    cout << "Snake Head: (" << headRect.x << ", " << headRect.y << "), Food: (" << foodRect.x << ", " << foodRect.y << ")" << endl;
    
    if (checkCollision(headRect, foodRect)) {
        // Add new segment at the position of the last segment
        Segment newSegment = {snake.back().x, snake.back().y, snakeBody, 0.0};
        snake.insert(snake.end() - 1, newSegment);

        // Place new food
        placeFood();
    }

    // Check for self-collision
    for (size_t i = 1; i < snake.size(); ++i) {
        SDL_Rect segmentRect = {snake[i].x, snake[i].y, snakeSegmentSize, snakeSegmentSize};
        if (checkCollision(headRect, segmentRect)) {
            isRunning = false;  // Game over
        }
    }
}

void render() {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 210, 126, 26, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    // Render food
    SDL_Rect foodRect = {food.x, food.y, snakeSegmentSize, snakeSegmentSize};
    SDL_RenderCopy(renderer, food.texture, NULL, &foodRect);

    // Render snake
    for (size_t i = 0; i < snake.size(); ++i) {
        SDL_Rect rect = {snake[i].x, snake[i].y, snakeSegmentSize, snakeSegmentSize};
        SDL_RenderCopyEx(renderer, snake[i].texture, NULL, &rect, snake[i].angle, NULL, SDL_FLIP_NONE);
    }

    // Present the renderer
    SDL_RenderPresent(renderer);
}

void cleanUp() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(snakeHead);
    SDL_DestroyTexture(snakeBody);
    SDL_DestroyTexture(snakeTail);
    SDL_DestroyTexture(foodTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(time(0))); // Seed the random number generator

    if (!initializeWindow()) {
        return -1;
    }

    isRunning = true;

    while (isRunning) {
        handleEvents();
        update();
        render();

        // Cap the frame rate to about 60 FPS
        SDL_Delay(16);
    }

    cleanUp();

    return 0;
}

