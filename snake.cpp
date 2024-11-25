#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDl2/SDL_mixer.h>
#include <iostream>
#include <sstream>
#include <bits/stdc++.h>
#include <string>
#include <cstdlib>
#include <ctime>
/***
 * 
 *                                             DEFINE
 * 
 ***/
using namespace std;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
int snakeSegmentSize = 24;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture *backgroundTexture = NULL;
SDL_Texture *backgroundTexture1 = NULL;
SDL_Texture *backgroundTexture2 = NULL;
SDL_Texture *themeBackground = NULL;
SDL_Texture *menuBackground = NULL;
SDL_Texture *gameOver = NULL;

SDL_Texture *snakeHead = NULL;
SDL_Texture *snakeBody = NULL;
SDL_Texture *snakeTail = NULL;

SDL_Texture *foodTexture = NULL;
SDL_Texture *bonusFoodTexture = NULL;
SDL_Texture *bonusFoodCover = NULL;

SDL_Texture *restartIcon = NULL;
SDL_Texture *backToMenu = NULL;
SDL_Texture *backButtonTexture = NULL;

SDL_Rect backBtnRect = {SCREEN_WIDTH / 2 + 80, SCREEN_HEIGHT / 2 + 220, 110, 35};
SDL_Rect helpBackBtnRect = {SCREEN_WIDTH / 2 + 80, SCREEN_HEIGHT / 2 + 227, 110, 35};
SDL_Rect deadBackBtnRect = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT / 2 + 95, 60, 45};

// level1
SDL_Rect obs1 = {417, 142, 118, 28};
SDL_Rect obs2 = {294, 442, 115, 25};
SDL_Rect obs3 = {346, 218, 24, 87};
SDL_Rect obs4 = {490, 328, 24, 95};

int obs5y=0;
int obs6y=0;
int obs7x=0;
SDL_Rect obs5 = {SCREEN_WIDTH/4,obs5y, snakeSegmentSize, snakeSegmentSize};
SDL_Rect obs6 = {SCREEN_WIDTH/4+SCREEN_WIDTH/2,obs6y, snakeSegmentSize, snakeSegmentSize};
SDL_Rect obs7 = {obs7x,SCREEN_HEIGHT/2, snakeSegmentSize, snakeSegmentSize};
// high score page
SDL_Texture *highScoreStar = NULL;
SDL_Texture *highScoreLabel = NULL;
SDL_Texture *highScoreNumberValue = NULL;

// popup score
SDL_Texture *scorePopup = NULL;
SDL_Texture *highScorePopup = NULL;
int popUpX = SCREEN_WIDTH / 2;
int bonusPopUpY = SCREEN_HEIGHT + 100;
int popUpY = SCREEN_HEIGHT + 100;
bool showPopup = false;
bool showBonusPopup = false;
Uint32 popupStartTime = 0;
const int popupDuration = 500;
const int popupInitialY = SCREEN_HEIGHT / 2;
const int popupSpeed = 5;

// helpsection
SDL_Texture *helpBoardTexture = NULL;

// menu button rect
SDL_Rect menuPlayRect = {SCREEN_WIDTH / 2 + 25, 190 - 1, 247, 46};
SDL_Rect menuHighScoreRect = {SCREEN_WIDTH / 2 + 25, 257, 247, 46};
SDL_Rect menuHelpRect = {SCREEN_WIDTH / 2 + 25, 325, 247, 46};
SDL_Rect menuExitRect = {SCREEN_WIDTH / 2 + 25, 395, 247, 46};

TTF_Font *font = NULL;

SDL_Texture *scoreTexture = NULL; 
SDL_Texture *scoreValueTexture = NULL;

// bgm and sound effect
Mix_Chunk *biteSound = NULL;
Mix_Chunk *mouseSound = NULL;
Mix_Chunk *clickSound = NULL;
Mix_Chunk *getPointFx = NULL;
Mix_Chunk *getMoneyFX = NULL;
Mix_Chunk *magicFX = NULL;
Mix_Chunk *gameOverFx = NULL;
Mix_Music *themeMusic = NULL;
Mix_Music *menuMusic = NULL;
Mix_Music *gameplayMusic = NULL;
Mix_Music *gameOverMusic = NULL;

bool isRunning;
bool isPaused = false;
bool isDead = false;

int score = 0;

Uint32 snakeSpeed = 200;
Uint32 lastMoveTime = 0;
// rotating enchinted food cover
double foodCoverRotationAngle = 0.0;
const double foodCoverRotationSpeed = 90.0;

Uint32 lastTime = SDL_GetTicks();
int highScore = 0;

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};
Direction dir = RIGHT;
enum Section
{
    THEME,
    MENU,
    GAMESCREEN,
    HELP,
    CHOSELEVEL,
    HIGHSCORE
};

int level = 0;
Section currentSection = THEME;
struct Segment
{
    int x, y;
    SDL_Texture *texture;
    double angle;
};

struct bonusFoods
{
    int x, y;
    SDL_Texture *texture;
    Uint32 spawnTime;
    bool isBonus;
};
bonusFoods bonusFood;
Uint32 bonusFoodAppearTime = 0;
bool bonusFoodVisible = false;

vector<Segment> snake;
struct Food
{
    int x, y;
    SDL_Texture *texture;
};
Food food;

// Function to load an image as a texture
SDL_Texture *loadTexture(const std::string &path, SDL_Renderer *renderer)
{
    SDL_Texture *newTexture = NULL;
    SDL_Surface *loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL)
    {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
    }
    else
    {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == NULL)
        {
            std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

// Function to render text  as  SDL_Texture
SDL_Texture *renderText(TTF_Font *font, const string &message, SDL_Color color, int &width, int &height)
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, message.c_str(), color);
    if (!textSurface)
    {
        cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << endl;
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    width = textSurface->w;  
    height = textSurface->h; 
    SDL_FreeSurface(textSurface);
    return texture;
}

void placeFood()
{
    bool validPosition;
    do
    {
        validPosition = true;
        food.x = (rand() % (SCREEN_WIDTH / snakeSegmentSize)) * snakeSegmentSize;
        food.y = (rand() % (SCREEN_HEIGHT / snakeSegmentSize)) * snakeSegmentSize;
        SDL_Rect foodObj = {food.x, food.y, snakeSegmentSize, snakeSegmentSize};
       
        if (food.x < 50 || food.x >= SCREEN_WIDTH - 50 || food.y < 50 || food.y >= SCREEN_HEIGHT - 50)
        {
            validPosition = false;
        }
        else if ((SDL_HasIntersection(&foodObj, &obs1) || SDL_HasIntersection(&foodObj, &obs2) || SDL_HasIntersection(&foodObj, &obs3) || SDL_HasIntersection(&foodObj, &obs4)))
        {
            validPosition = false;
        }
        else
        {
            for (const Segment &segment : snake)
            {
                if (food.x == segment.x && food.y == segment.y)
                {
                    validPosition = false;
                    break;
                }
            }
        }
    } while (!validPosition);
}

void placeBonusFood()
{
    bool validPosition;
    do
    {
        validPosition = true;
        //Random grid position
        bonusFood.x = (rand() % (SCREEN_WIDTH / snakeSegmentSize)) * snakeSegmentSize;
        bonusFood.y = (rand() % (SCREEN_HEIGHT / snakeSegmentSize)) * snakeSegmentSize;

        //Boundary Cheak
        if (bonusFood.x < 50 || bonusFood.x >= SCREEN_WIDTH - 50 || bonusFood.y < 50 || bonusFood.y >= SCREEN_HEIGHT - 50)
        {
            validPosition = false;
        }
        else
        {
            for (const Segment &segment : snake)
            {
                if (bonusFood.x == segment.x && bonusFood.y == segment.y)
                {
                    validPosition = false;
                    break;
                }
            }

        }
    } while (!validPosition);
}
/***
 * 
 *                                              INITALIZATION SECTION
 * 
 ***/
bool initializeWindow()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        cout << "Error: SDL initialization failed\nSDL Error: " << SDL_GetError() << endl;
        isRunning = false;
        return false;
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        cout << "Error: Failed to create window\nSDL Error: " << SDL_GetError() << endl;
        isRunning = false;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        cout << "Error: Failed to create renderer\nSDL Error: " << SDL_GetError() << endl;
        isRunning = false;
        return false;
    }
    if (TTF_Init() == -1)
    {
        cout << "Error: ttf init failed" << TTF_GetError() << endl;
        isRunning == false;
        return false;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        cout << "Error: Failed to initialize SDL_image\nSDL_image Error: " << IMG_GetError() << endl;
        isRunning = false;
        return false;
    }
    font = TTF_OpenFont("resource/pg.ttf", 36);
    if (!font)
    {
        cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << endl;
        isRunning = false;
        return false;
    }
    // Load background texture
    backgroundTexture1 = loadTexture("resource/g1.png", renderer);
    if (!backgroundTexture1)
    {
        cout << "Error: Failed to load background texture" << endl;
        isRunning = false;
        return false;
    }
    backgroundTexture2 = loadTexture("resource/g2.png", renderer);
    if (!backgroundTexture2)
    {
        cout << "Error: Failed to load background texture" << endl;
        isRunning = false;
        return false;
    }
    backgroundTexture = backgroundTexture1;
    // Load snake head texture
    snakeHead = loadTexture("resource/head.png", renderer);
    if (!snakeHead)
    {
        cout << "Error: Failed to load snake head texture" << endl;
        isRunning = false;
        return false;
    }

    // Load snake body texture
    snakeBody = loadTexture("resource/body.png", renderer);
    if (!snakeBody)
    {
        cout << "Error: Failed to load snake body texture" << endl;
        isRunning = false;
        return false;
    }

    // Load snake tail texture
    snakeTail = loadTexture("resource/tail.png", renderer);
    if (!snakeTail)
    {
        cout << "Error: Failed to load snake tail texture" << endl;
        isRunning = false;
        return false;
    }

    // Load food texture
    foodTexture = loadTexture("resource/food.png", renderer);
    if (!foodTexture)
    {
        cout << "Error: Failed to load food texture" << endl;
        isRunning = false;
        return false;
    }

    // gameover png
    gameOver = loadTexture("resource/game-over.png", renderer);
    if (!gameOver)
    {
        cout << "Error: Failed to load play texture" << endl;
        isRunning = false;
        return false;
    }
    // Load play icon
    restartIcon = loadTexture("resource/game-over.png", renderer);
    if (!gameOver)
    {
        cout << "Error: Failed to load play texture" << endl;
        isRunning = false;
        return false;
    }

    // theme background
    themeBackground = loadTexture("resource/theme.png", renderer);
    if (!themeBackground)
    {
        cout << "Error: Failed to theme background texture" << endl;
        isRunning = false;
        return false;
    }

    // menu
    menuBackground = loadTexture("resource/menu.png", renderer);
    if (!menuBackground)
    {
        cout << "Error: Failed to menu background texture" << endl;
        isRunning = false;
        return false;
    }

    bonusFoodCover = loadTexture("resource/glower.png", renderer);
    if (!bonusFoodCover)
    {
        cout << "Error: Failed to bonusFoodCover texture" << endl;
        isRunning = false;
        return false;
    }
    bonusFoodTexture = loadTexture("resource/enchinted-apple.png", renderer);
    if (!bonusFoodTexture)
    {
        cout << "Error: Failed to exit texture" << endl;
        isRunning = false;
        return false;
    }

    // high score page
    highScoreStar = loadTexture("resource/star.png", renderer);
    if (!highScoreStar)
    {
        cout << "Error: Failed to star texture" << endl;
        isRunning = false;
        return false;
    }
    helpBoardTexture = loadTexture("resource/guideline.png", renderer);
    if (!helpBoardTexture)
    {
        cout << "Error: Failed to guideline texture" << endl;
        isRunning = false;
        return false;
    }
    backToMenu = loadTexture("resource/back.png", renderer);
    if (!backToMenu)
    {
        cout << "Error: Failed to back to menu texture" << endl;
        isRunning = false;
        return false;
    }
    bonusFood.texture = bonusFoodTexture;
    // Initialize snake
    snake.push_back({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, snakeHead, 0.0});
    snake.push_back({SCREEN_WIDTH / 2 - snakeSegmentSize, SCREEN_HEIGHT / 2, snakeBody, 0.0});
    snake.push_back({SCREEN_WIDTH / 2 - 2 * snakeSegmentSize, SCREEN_HEIGHT / 2, snakeTail, 0.0});

    // Initialize food
    food.texture = foodTexture;
    placeFood();

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        cout << "Error: Failed to initialize SDL_mixer\nMix Error: " << Mix_GetError() << endl;
        isRunning = false;
        return false;
    }

    biteSound = Mix_LoadWAV("resource/bite.wav");
    mouseSound = Mix_LoadWAV("resource/mouseclick.wav");
    clickSound = Mix_LoadWAV("resource/click.wav");
    getPointFx = Mix_LoadWAV("resource/point.wav");
    getMoneyFX = Mix_LoadWAV("resource/cash.wav");
    gameOverFx = Mix_LoadWAV("resource/gameoversound.wav");
    magicFX = Mix_LoadWAV("resource/magic.wav");
    if (!biteSound || !mouseSound || !clickSound || !getPointFx || !getMoneyFX || !gameOverFx || !magicFX)
    {
        cout << "Failed to load scratch sound effect! SDL_mixer Error: " << Mix_GetError() << endl;
        return false;
    }

    themeMusic = Mix_LoadMUS("resource/theme.mp3");
    menuMusic = Mix_LoadMUS("resource/menu.mp3");
    gameplayMusic = Mix_LoadMUS("resource/gameplay.mp3");
    gameOverMusic = Mix_LoadMUS("resource/gameover.mp3");
    if (!themeMusic || !menuMusic || !gameplayMusic || !gameOverMusic)
    {
        std::cout << "Failed to load music! SDL_mixer Error: " << Mix_GetError() << endl;
        return false;
    }

    return true;
}

// store high score
//  Function declarations
void saveHighScore(const char *filename, int highScore)
{
    FILE *file = fopen(filename, "w");
    if (file != nullptr)
    {
        fprintf(file, "%d", highScore);
        fclose(file);
    }
    else
    {
        std::cout << "Unable to open file to write high score" << std::endl;
    }
}
void loadHighScore(const char *filename, int &highScore)
{
    FILE *file = fopen(filename, "r");
    if (file != nullptr)
    {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
    else
    {
        std::cout << "File does not exist. Creating a new high score file." << std::endl;
        highScore = 0;
        saveHighScore(filename, highScore);
    }
}

/***
 * 
 *                                              EVENT SECTION
 * 
 ***/
//-----------------------------------------THEME-----------------------------------------
void themeHandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            isRunning = false;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_RETURN)
            {
                Mix_PlayChannel(-1, mouseSound, 0);
                currentSection = MENU;
            }
        }
    }
}
//------------------------------SCORE EVENT---------------------------------------------
void scorePageHandle()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            isRunning = false;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            int x, y;
            SDL_GetMouseState(&x, &y);

            if (x >= backBtnRect.x && x <= backBtnRect.x + backBtnRect.w &&
                y >= backBtnRect.y && y <= backBtnRect.y + backBtnRect.h)
            {
                Mix_PlayChannel(-1, mouseSound, 0);
                currentSection = MENU;
            }
        }
    }
}
//--------------------------------------HELP EVENT-------------------------------------------------
void helpPageHandle()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            isRunning = false;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            int x, y;
            SDL_GetMouseState(&x, &y);

            if (x >= helpBackBtnRect.x && x <= helpBackBtnRect.x + helpBackBtnRect.w &&
                y >= helpBackBtnRect.y && y <= helpBackBtnRect.y + helpBackBtnRect.h)
            {
                Mix_PlayChannel(-1, mouseSound, 0);

                currentSection = MENU;
            }
        }
    }
}
void menuHandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            isRunning = false;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_RETURN)
            {
                currentSection = GAMESCREEN;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            int x, y;
            SDL_GetMouseState(&x, &y);

            if (x >= menuPlayRect.x && x <= menuPlayRect.x + menuPlayRect.w &&
                y >= menuPlayRect.y && y <= menuPlayRect.y + menuPlayRect.h)
            {
                Mix_PlayChannel(-1, mouseSound, 0);
                currentSection = GAMESCREEN;
            }
            else if (x >= menuExitRect.x && x <= menuExitRect.x + menuExitRect.w &&
                     y >= menuExitRect.y && y <= menuExitRect.y + menuExitRect.h)
            {
                Mix_PlayChannel(-1, mouseSound, 0);
                isRunning = false;
            }
            else if (x >= menuHelpRect.x && x <= menuHelpRect.x + menuHelpRect.w &&
                     y >= menuHelpRect.y && y <= menuHelpRect.y + menuHelpRect.h)
            {
                Mix_PlayChannel(-1, mouseSound, 0);
                currentSection = HELP;
            }
            else if (x >= menuHighScoreRect.x && x <= menuHighScoreRect.x + menuHighScoreRect.w &&
                     y >= menuHighScoreRect.y && y <= menuHighScoreRect.y + menuHighScoreRect.h)
            {
                Mix_PlayChannel(-1, mouseSound, 0);
                currentSection = HIGHSCORE;
            }
        }
    }
}

/***
 * 
 *                                              RESET THE GAME 
 * 
 ***/
void reset(){
    score=0; 
    snake.clear();
    snake.push_back({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, snakeHead, 0.0});
    snake.push_back({SCREEN_WIDTH / 2 - snakeSegmentSize, SCREEN_HEIGHT / 2, snakeBody, 0.0});
    snake.push_back({SCREEN_WIDTH / 2 - 2 * snakeSegmentSize, SCREEN_HEIGHT / 2, snakeTail, 0.0});
    placeFood();
    isPaused=false;
    isDead=false;
    Direction dir = RIGHT;
    backgroundTexture=backgroundTexture1;
    level=0;
}
void handleEvents()
{

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            isRunning = false;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            // kbd-press
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                if (!isPaused && dir != DOWN)
                    dir = UP;
                break;
            case SDLK_DOWN:
                if (!isPaused && dir != UP)
                    dir = DOWN;
                break;
            case SDLK_LEFT:
                if (!isPaused && dir != RIGHT)
                    dir = LEFT;
                break;
            case SDLK_RIGHT:
                if (!isPaused && dir != LEFT)
                    dir = RIGHT;
                break;
            case SDLK_SPACE:
                if (!isPaused)
                {
                    isPaused = true;
                }
                else
                {
                    isPaused = false;
                }
                break;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            int x, y;
            SDL_GetMouseState(&x, &y);

            if (isDead)
            {
                if (x >= deadBackBtnRect.x && x <= deadBackBtnRect.x + deadBackBtnRect.w &&
                    y >= deadBackBtnRect.y && y <= deadBackBtnRect.y + deadBackBtnRect.h)
                {
                    Mix_PlayChannel(-1, mouseSound, 0);
                    reset();

                    currentSection = MENU;
                }
            }
        }
    }
}

bool checkCollision(const SDL_Rect &a, const SDL_Rect &b)
{
    return SDL_HasIntersection(&a, &b);
}
/***
 * 
 *                                              UPDATE SECTION
 * 
 ***/

void update()
{
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastMoveTime < snakeSpeed)
    {
        return;
    }
    lastMoveTime = currentTime;
    // food cover rotation
    double deltaTime = (currentTime - lastTime) / 1000.0;
    foodCoverRotationAngle += foodCoverRotationSpeed * deltaTime;
    if (foodCoverRotationAngle >= 360.0)
        foodCoverRotationAngle -= 360.0;
    lastTime = currentTime;
if(level==2){
static int obs5Speed = 10; 
obs5y += obs5Speed;

if (obs5y <= 0 || obs5y >= SCREEN_HEIGHT - snakeSegmentSize) {
    obs5Speed = -obs5Speed; 
}

static int obs6Speed = 15; 
obs6y += obs6Speed;

if (obs6y <= 0 || obs6y >= SCREEN_HEIGHT - snakeSegmentSize) {
    obs6Speed = -obs6Speed; 
}

static int obs7Speed = 10; 
obs7x += obs7Speed;

if (obs7x <= 0 || obs7x>= SCREEN_HEIGHT - snakeSegmentSize) {
    obs7Speed= -obs7Speed; 
}
}

    // Update snake segments positions
    for (int i = snake.size() - 1; i > 0; --i)
    {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
        snake[i].angle = snake[i - 1].angle;
    }

    // Update head position
    switch (dir)
    {
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
    if (snake[0].x < 0)
        snake[0].x = SCREEN_WIDTH - snakeSegmentSize;
    if (snake[0].x >= SCREEN_WIDTH)
        snake[0].x = 0;
    if (snake[0].y < 0)
        snake[0].y = SCREEN_HEIGHT - snakeSegmentSize;
    if (snake[0].y >= SCREEN_HEIGHT)
        snake[0].y = 0;

    // Check for food collision
    SDL_Rect headRect = {snake[0].x, snake[0].y, snakeSegmentSize, snakeSegmentSize};
    SDL_Rect foodRect = {food.x, food.y, snakeSegmentSize, snakeSegmentSize};
    if (checkCollision(headRect, foodRect))
    {

        // Add new segment at the position of the last segment
        Segment newSegment = {snake.back().x, snake.back().y, snakeBody, 0.0};
        snake.insert(snake.end() - 1, newSegment);
        score += 10;
        Mix_PlayChannel(-1, getPointFx, 0);

        // Show score popup
        showPopup = true;
        popupStartTime = currentTime; // Record the start time
        popUpY = popupInitialY;       // Reset popup Y position

        // Place new food
        placeFood();
    }

    if (showPopup)
    {
        Uint32 elapsedTime = currentTime - popupStartTime;

        if (elapsedTime <= popupDuration)

        {
            // Adjust the popup movement: assume we want it to move up by 40 pixels over 0.8 seconds
            float progress = static_cast<float>(elapsedTime) / popupDuration; // Progress from 0.0 to 1.0
            popUpY = popupInitialY - (progress * 40);                         // Move the popup up by 40 pixels
        }
        else
        {
            popUpY = -100;
            showPopup = false; // Hide the popup after the duration
        }
    }
    // cheak bonus food
    if (bonusFoodVisible)
    {

        SDL_Rect bonusFoodRect = {bonusFood.x, bonusFood.y, snakeSegmentSize, snakeSegmentSize};
        if (checkCollision(headRect, bonusFoodRect))
        {
            // Add new segment at the position of the last segment
            Segment newSegment = {snake.back().x, snake.back().y, snakeBody, 0.0};
            snake.insert(snake.end() - 1, newSegment);
            score += 50; // Bonus food gives more points
            Mix_PlayChannel(-1, getMoneyFX, 0);
            // Show score popup
            showBonusPopup = true;
            popupStartTime = currentTime; // Record the start time
            bonusPopUpY = popupInitialY;  // Reset popup Y position

            // Hide bonus food
            bonusFoodVisible = false;
        }
    }

    if (showBonusPopup)
    {
        Uint32 elapsedTime = currentTime - popupStartTime;

        if (elapsedTime <= popupDuration)

        {
            
            float progress = static_cast<float>(elapsedTime) / popupDuration; // Progress from 0.0 to 1.0
            bonusPopUpY = popupInitialY - (progress * 40);                  
        }
        else
        {
            bonusPopUpY = -100;
            showBonusPopup = false;
        }
    }

    if (!bonusFoodVisible && (rand() % 100 < 5)) // 5% chance to spawn bonus food each update&& (rand() % 100 < 5)
    {
        placeBonusFood();
        Mix_PlayChannel(-1, magicFX, 0);
        bonusFoodVisible = true;
        bonusFoodAppearTime = currentTime;
    }
    if (bonusFoodVisible && currentTime - bonusFoodAppearTime > 5000) // Bonus food stays for 5 seconds
    {
        bonusFoodVisible = false;
    }

    if (score > highScore)
    {
        highScore = score;
        saveHighScore("high-score.txt", highScore);
    }

    // Check for self-collision
    for (size_t i = 1; i < snake.size(); ++i)
    {
        SDL_Rect segmentRect = {snake[i].x, snake[i].y, snakeSegmentSize, snakeSegmentSize};
        if (checkCollision(headRect, segmentRect))
        {
            isPaused = true;
            Mix_PlayChannel(-1, gameOverFx, 0);
            isDead = true;
            // Game over
        }
    }


    if ((snake[0].x < 36 || snake[0].x >= SCREEN_WIDTH - 36) )
    {
        Mix_PlayChannel(-1, gameOverFx, 0);
        isPaused = true;
        isDead = true;
    }




    

    if ((level == 1) && (checkCollision(headRect, obs1) || checkCollision(headRect, obs2) || checkCollision(headRect, obs1) || checkCollision(headRect, obs3) || checkCollision(headRect, obs4)))
    {
        Mix_PlayChannel(-1, gameOverFx, 0);
        isPaused = true;
        isDead = true;
    }

    if ((level == 2) && (checkCollision(headRect, obs5) || checkCollision(headRect, obs6) || checkCollision(headRect, obs7) ))
    {
        Mix_PlayChannel(-1, gameOverFx, 0);
        isPaused = true;
        isDead = true;
    }

    if (level == 0 && score > 100)
    {
        backgroundTexture = backgroundTexture2;
        level = 1;
        cout << "level 1 is loading" << endl;
    }
    if (level == 1 && score > 200)
    {
        backgroundTexture = backgroundTexture1;
        level = 2;
        cout << "level 2 is loading" << endl;
    }
}

/***
 * 
 *                                              Render Section
 * 
 ***/
// --------------------------------------Theme Render -------------------------------------------------
void themeRender()
{
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 210, 126, 26, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_RenderCopy(renderer, themeBackground, NULL, NULL);

    // Present the renderer
    SDL_RenderPresent(renderer);
}
// --------------------------------------Menu Render --------------------------------------------------
void menuRender()
{
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 210, 126, 26, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_RenderCopy(renderer, menuBackground, NULL, NULL);

    // Present the renderer
    SDL_RenderPresent(renderer);
}
// --------------------------------------High Score Render --------------------------------------------------
void highScoreRender()
{
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 210, 126, 26, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_RenderCopy(renderer, menuBackground, NULL, NULL);

    SDL_RenderCopy(renderer, highScoreStar, NULL, NULL);
    // render score board
    ostringstream highScoreNumber;
    highScoreNumber << highScore;
    string highScoreValue = highScoreNumber.str();
    // render
    SDL_Color textColor = {0, 0, 0};
    int highScoreValueTextW, highScoreValueTextH;

    highScoreNumberValue = renderText(font, highScoreValue, textColor, highScoreValueTextW, highScoreValueTextH);
    SDL_Rect scoreValueRect = {SCREEN_WIDTH / 2 + 105, SCREEN_HEIGHT / 2 - 45, highScoreValueTextW, highScoreValueTextH};

    SDL_RenderCopy(renderer, highScoreNumberValue, NULL, &scoreValueRect);

    // Present the renderer
    SDL_RenderPresent(renderer);
}
// --------------------------------------Help Render --------------------------------------------------
void helpRender()
{
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 210, 126, 26, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_RenderCopy(renderer, menuBackground, NULL, NULL);
    SDL_RenderCopy(renderer, helpBoardTexture, NULL, NULL);

    // Present the renderer
    SDL_RenderPresent(renderer);
}
// --------------------------------------Main Render --------------------------------------------------
void render()
{
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 210, 126, 26, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    // obs
    obs5 ={SCREEN_WIDTH/4,obs5y,snakeSegmentSize,snakeSegmentSize};
    obs6 ={SCREEN_WIDTH/4+SCREEN_WIDTH/2,obs6y,snakeSegmentSize,snakeSegmentSize};
    obs7 = {obs7x,SCREEN_HEIGHT/2, snakeSegmentSize, snakeSegmentSize};
    SDL_SetRenderDrawColor(renderer, 0,0, 0, 255);
  
    if(level==2){
    SDL_RenderFillRect(renderer, &obs5);
    SDL_RenderFillRect(renderer, &obs6);
    SDL_RenderFillRect(renderer, &obs7);
    }
    
    // Render food
    SDL_Rect foodRect = {food.x, food.y, snakeSegmentSize, snakeSegmentSize};
    SDL_RenderCopy(renderer, food.texture, NULL, &foodRect);

    // Render bonus food if visible
    if (bonusFoodVisible)
    {
        SDL_Rect bonusFoodRect = {bonusFood.x, bonusFood.y, snakeSegmentSize, snakeSegmentSize};
        SDL_RenderCopy(renderer, bonusFood.texture, NULL, &bonusFoodRect);
        SDL_Rect bonusFoodRectCover = {bonusFood.x - 10, bonusFood.y - 10, snakeSegmentSize + 20, snakeSegmentSize + 20};
        SDL_Point center = {bonusFoodRectCover.w / 2, bonusFoodRectCover.h / 2}; 

        SDL_RenderCopyEx(renderer, bonusFoodCover, NULL, &bonusFoodRectCover, foodCoverRotationAngle, &center, SDL_FLIP_NONE);
    }

    // Render snake
    for (size_t i = 0; i < snake.size(); ++i)
    {
        SDL_Rect rect = {snake[i].x, snake[i].y, snakeSegmentSize, snakeSegmentSize};
        SDL_RenderCopyEx(renderer, snake[i].texture, NULL, &rect, snake[i].angle, NULL, SDL_FLIP_NONE);
    }
    // render score board
    ostringstream scoreNumber;
    scoreNumber << score;
    string scoreValue = scoreNumber.str();
    // render
    SDL_Color textColor = {0, 0, 0};
    int scoreValueTextW, scoreValueTextH;
    scoreValueTexture = renderText(font, scoreValue, textColor, scoreValueTextW, scoreValueTextH);

    SDL_Rect scoreValueRect = {SCREEN_WIDTH / 2, 15, scoreValueTextW, scoreValueTextH};
    SDL_RenderCopy(renderer, scoreValueTexture, NULL, &scoreValueRect);

    SDL_Color textPopColor = {0, 0, 0};
    int scorePopW, scorePopH, highScorePopW, highScorePopH;
    scorePopup = renderText(font, "+10", textPopColor, scorePopW, scorePopH);
    highScorePopup = renderText(font, "+50", textPopColor, highScorePopW, highScorePopH);

    SDL_Rect scorePopupRect = {popUpX, popUpY, scorePopW, scorePopH};
    SDL_Rect highScorePopupValueRect = {popUpX + 50, bonusPopUpY + 10, highScorePopW, highScorePopH};
    SDL_RenderCopy(renderer, scorePopup, NULL, &scorePopupRect);
    SDL_RenderCopy(renderer, highScorePopup, NULL, &highScorePopupValueRect);

    // game over screen
    if (isDead)
    {
        SDL_RenderCopy(renderer, gameOver, NULL, NULL);
        SDL_RenderCopy(renderer, backToMenu, NULL, &deadBackBtnRect);

        SDL_Event event;
    }

    // Present the renderer
    SDL_RenderPresent(renderer);
}

void cleanUp()
{
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(snakeHead);
    SDL_DestroyTexture(snakeBody);
    SDL_DestroyTexture(snakeTail);
    SDL_DestroyTexture(foodTexture);
    SDL_DestroyTexture(scoreValueTexture);

    SDL_DestroyTexture(gameOver);
    SDL_DestroyTexture(themeBackground);
    SDL_DestroyTexture(menuBackground);

    SDL_DestroyTexture(bonusFoodCover);
    SDL_DestroyTexture(bonusFoodTexture);
    SDL_DestroyTexture(highScoreStar);
    SDL_DestroyTexture(highScoreLabel);
    SDL_DestroyTexture(highScoreNumberValue);
    SDL_DestroyTexture(helpBoardTexture);
    SDL_DestroyTexture(backToMenu);
    // clear music
    Mix_FreeChunk(biteSound);
    biteSound = NULL;
    Mix_FreeChunk(mouseSound);
    mouseSound = NULL;
    Mix_FreeChunk(clickSound);
    clickSound = NULL;
    Mix_FreeMusic(gameplayMusic);
    gameplayMusic = NULL;
    Mix_FreeMusic(gameOverMusic);
    gameOverMusic = NULL;
    Mix_FreeMusic(themeMusic);
    themeMusic = NULL;
    Mix_FreeMusic(menuMusic);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char **argv)
{
    srand(static_cast<unsigned int>(time(0))); 

    if (!initializeWindow())
    {
        return -1;
    }

    isRunning = true;
    auto currenMusic = themeMusic;

    // Start playing the initial music
    Mix_PlayMusic(currenMusic, -1);

    loadHighScore("high-score.txt", highScore);

    while (isRunning)
    {
        switch (currentSection)
        {
        case THEME:
            if (currenMusic != themeMusic)
            {
                currenMusic = themeMusic;
                Mix_PlayMusic(currenMusic, -1);
            }
            themeHandleEvents();
            themeRender();
            break;

        case MENU:
            if (currenMusic != menuMusic)
            {
                currenMusic = menuMusic;
                Mix_PlayMusic(currenMusic, -1);
            }
            menuHandleEvents();
            menuRender();
            break;
        case HIGHSCORE:
            //  Mix_PlayMusic(currenMusic, -1);
            scorePageHandle();
            highScoreRender();
            break;
        case HELP:
            helpPageHandle();
            helpRender();
            break;

        case GAMESCREEN:

            handleEvents();
            if (!isPaused)
                update();
            render();
            if (!isDead)
            {
                if (currenMusic != gameplayMusic)
                {
                    currenMusic = gameplayMusic;
                    Mix_PlayMusic(currenMusic, -1);
                }
            }
            else
            {
                if (currenMusic != menuMusic)
                {
                    currenMusic = menuMusic;
                    Mix_PlayMusic(currenMusic, -1);
                }
            }
            break;

        default:
            break;
        }

        SDL_Delay(16);//60 fps
    }
    cleanUp();

    return 0;
}
