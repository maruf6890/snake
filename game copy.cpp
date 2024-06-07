#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <ctime>

using namespace std;

// Screen dimensions
const int SCREEN_WIDTH = 854;
const int SCREEN_HEIGHT = 530;
const int SNAKE_SIZE = 40; // Increased size
const int SNAKE_SPEED = 10; // Decreased speed

// Menu options
enum MenuOption {
    CONTINUE_GAME,
    NEW_GAME,
    HIGH_SCORE,
    QUIT,
    NONE
};

// Direction constants
const int UP = 0;
const int DOWN = 1;
const int LEFT = 2;
const int RIGHT = 3;

int currentDirection = RIGHT;
bool running;

struct SnakeSegment {
    int x, y;
    int direction;
};

vector<SnakeSegment> snake;
SDL_Rect food;
bool foodEaten;

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

void showInstructions(SDL_Renderer *renderer) {
    SDL_Texture *background = loadTexture("resource/theme.png", renderer);
    if (background == NULL) {
        return;
    }

    TTF_Font *font = TTF_OpenFont("resource/pg.ttf", 18);
    if (font == NULL) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_DestroyTexture(background);
        return;
    }

    SDL_Color textColor = {0, 0, 0};
    SDL_Surface *instructionsSurface = TTF_RenderText_Solid(font, "Press Enter to Continue", textColor);
    if (instructionsSurface == NULL) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        SDL_DestroyTexture(background);
        return;
    }

    SDL_Texture *instructionsTexture = SDL_CreateTextureFromSurface(renderer, instructionsSurface);
    SDL_FreeSurface(instructionsSurface);
    if (instructionsTexture == NULL) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        TTF_CloseFont(font);
        SDL_DestroyTexture(background);
        return;
    }

    SDL_Rect instructionsRect = {SCREEN_WIDTH / 2 - instructionsSurface->w / 2, SCREEN_HEIGHT / 2 + 100, instructionsSurface->w, instructionsSurface->h};

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, NULL, NULL);
    SDL_RenderCopy(renderer, instructionsTexture, NULL, &instructionsRect);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(background);
    SDL_DestroyTexture(instructionsTexture);
    TTF_CloseFont(font);

    bool continueFlag = false;
    SDL_Event e;
    while (!continueFlag) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                exit(0);
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                continueFlag = true;
            }
        }
    }
}

MenuOption showMenu(SDL_Renderer *renderer) {
    SDL_Texture *background = loadTexture("resource/theme2.png", renderer);
    if (background == NULL) {
        return QUIT;
    }

    TTF_Font *font = TTF_OpenFont("resource/pg.ttf", 18);
    if (font == NULL) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_DestroyTexture(background);
        return QUIT;
    }

    SDL_Color textColor = {0, 0, 0};

    SDL_Surface *continueSurface = TTF_RenderText_Solid(font, "Continue", textColor);
    SDL_Surface *newGameSurface = TTF_RenderText_Solid(font, "New Game", textColor);
    SDL_Surface *highScoreSurface = TTF_RenderText_Solid(font, "High Score", textColor);
    SDL_Surface *quitSurface = TTF_RenderText_Solid(font, "Quit", textColor);

    if (continueSurface == NULL || newGameSurface == NULL || highScoreSurface == NULL || quitSurface == NULL) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_DestroyTexture(background);
        TTF_CloseFont(font);
        return QUIT;
    }

    SDL_Texture *continueTexture = SDL_CreateTextureFromSurface(renderer, continueSurface);
    SDL_Texture *newGameTexture = SDL_CreateTextureFromSurface(renderer, newGameSurface);
    SDL_Texture *highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
    SDL_Texture *quitTexture = SDL_CreateTextureFromSurface(renderer, quitSurface);

    SDL_FreeSurface(continueSurface);
    SDL_FreeSurface(newGameSurface);
    SDL_FreeSurface(highScoreSurface);
    SDL_FreeSurface(quitSurface);

    if (continueTexture == NULL || newGameTexture == NULL || highScoreTexture == NULL || quitTexture == NULL) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(background);
        SDL_DestroyTexture(continueTexture);
        SDL_DestroyTexture(newGameTexture);
        SDL_DestroyTexture(highScoreTexture);
        SDL_DestroyTexture(quitTexture);
        TTF_CloseFont(font);
        return QUIT;
    }

    SDL_Rect continueRect = {SCREEN_WIDTH / 2 - 100, 100, 120, 30};
    SDL_Rect newGameRect = {SCREEN_WIDTH / 2 - 100, 150, 200, 50};
    SDL_Rect highScoreRect = {SCREEN_WIDTH / 2 - 100, 200, 200, 50};
    SDL_Rect quitRect = {SCREEN_WIDTH / 2 - 100, 250, 200, 50};

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, NULL, NULL);
    SDL_RenderCopy(renderer, continueTexture, NULL, &continueRect);
    SDL_RenderCopy(renderer, newGameTexture, NULL, &newGameRect);
    SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);
    SDL_RenderCopy(renderer, quitTexture, NULL, &quitRect);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(background);
    SDL_DestroyTexture(continueTexture);
    SDL_DestroyTexture(newGameTexture);
    SDL_DestroyTexture(highScoreTexture);
    SDL_DestroyTexture(quitTexture);
    TTF_CloseFont(font);

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                return QUIT;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= continueRect.x && x <= continueRect.x + continueRect.w &&
                    y >= continueRect.y && y <= continueRect.y + continueRect.h) {
                    return CONTINUE_GAME;
                }
                if (x >= newGameRect.x && x <= newGameRect.x + newGameRect.w &&
                    y >= newGameRect.y && y <= newGameRect.y + newGameRect.h) {
                    return NEW_GAME;
                }
                if (x >= highScoreRect.x && x <= highScoreRect.x + highScoreRect.w &&
                    y >= highScoreRect.y && y <= highScoreRect.y + highScoreRect.h) {
                    return HIGH_SCORE;
                }
                if (x >= quitRect.x && x <= quitRect.x + quitRect.w &&
                    y >= quitRect.y && y <= quitRect.y + quitRect.h) {
                    return QUIT;
                }
            }
        }
    }
    return NONE;
}

void spawnFood() {
    bool validPosition = false;
    while (!validPosition) {
        food.x = (rand() % (SCREEN_WIDTH / SNAKE_SIZE)) * SNAKE_SIZE;
        food.y = (rand() % (SCREEN_HEIGHT / SNAKE_SIZE)) * SNAKE_SIZE;
        food.w = SNAKE_SIZE; // Increased size
        food.h = SNAKE_SIZE; // Increased size

        validPosition = true;
        for (auto segment : snake) {
            if (segment.x == food.x && segment.y == food.y) {
                validPosition = false;
                break;
            }
        }
    }
    cout << "Food spawned at (" << food.x << ", " << food.y << ")\n";
}

void update() {
    SnakeSegment newHead = snake[0];

    switch (currentDirection) {
        case UP:
            newHead.y -= SNAKE_SPEED;
            break;
        case DOWN:
            newHead.y += SNAKE_SPEED;
            break;
        case LEFT:
            newHead.x -= SNAKE_SPEED;
            break;
        case RIGHT:
            newHead.x += SNAKE_SPEED;
            break;
    }

    if (newHead.x < 0) newHead.x = SCREEN_WIDTH - SNAKE_SIZE;
    if (newHead.x >= SCREEN_WIDTH) newHead.x = 0;
    if (newHead.y < 0) newHead.y = SCREEN_HEIGHT - SNAKE_SIZE;
    if (newHead.y >= SCREEN_HEIGHT) newHead.y = 0;

    // Check if the snake eats food
    if (newHead.x == food.x && newHead.y == food.y) {
        // Increase snake size
        snake.push_back(snake.back());
        // Spawn new food
        spawnFood();
    } else {
        // Move snake by adding the new head
        snake.pop_back();
    }

    // Update snake head position
    snake[0] = newHead;

    // Check for collision with itself
    for (size_t i = 1; i < snake.size(); ++i) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            cout << "Collision detected at (" << snake[0].x << ", " << snake[0].y << ")\n";
            running = false;
            return;
        }
    }
}

void renderGame(SDL_Renderer *renderer, SDL_Texture *playGround, SDL_Texture *foodTexture) {
    SDL_RenderClear(renderer);

    // Render background
    SDL_Rect backgroundImage = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, playGround, NULL, &backgroundImage);

    // Render snake body
    for (size_t i = 0; i < snake.size(); ++i) {
        SDL_Rect bodyRect = {snake[i].x, snake[i].y, SNAKE_SIZE, SNAKE_SIZE};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green color for snake body
        SDL_RenderFillRect(renderer, &bodyRect);
    }

    // Render food
    SDL_RenderCopy(renderer, foodTexture, NULL, &food);

    SDL_RenderPresent(renderer);
}

void close(SDL_Window *window, SDL_Renderer *renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char *args[]) {
    srand(time(0));

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow("Snake Game",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // Load textures
    SDL_Texture *playGround = loadTexture("resource/g1.png", renderer);
    SDL_Texture *foodTexture = loadTexture("resource/food.png", renderer);

    if (!playGround || !foodTexture) {
        close(window, renderer);
        return -1;
    }

    // Show instructions
    showInstructions(renderer);

    // Main loop flag
    bool quit = false;

    // While application is running
    while (!quit) {
        // Show menu and get user choice
        MenuOption choice = showMenu(renderer);

        switch (choice) {
            case CONTINUE_GAME:
                running = true;
                break;
            case NEW_GAME:
    // Initialize snake
    snake.clear();
    snake.push_back({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, RIGHT});
    currentDirection = RIGHT;
    // Set running to true after initializing the snake
    running = true;
    // Spawn initial food
    spawnFood();
    break;
            case HIGH_SCORE:
                std::cout << "High Score selected" << std::endl;
                continue;
            case QUIT:
                quit = true;
                continue;
            default:
                continue;
        }

        // Main game loop
        while (!quit && running) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                } else if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_UP:
                            if (currentDirection != DOWN)
                                currentDirection = UP;
                            break;
                        case SDLK_DOWN:
                            if (currentDirection != UP)
                                currentDirection = DOWN;
                            break;
                        case SDLK_LEFT:
                            if (currentDirection != RIGHT)
                                currentDirection = LEFT;
                            break;
                        case SDLK_RIGHT:
                            if (currentDirection != LEFT)
                                currentDirection = RIGHT;
                            break;
                    }
                }
            }

            update();

            renderGame(renderer, playGround, foodTexture);

            SDL_Delay(100);
        }
    }

    SDL_DestroyTexture(playGround);
    SDL_DestroyTexture(foodTexture);
    close(window, renderer);

    return 0;
}
