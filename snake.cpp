#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDl2/SDL_mixer.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *backgroundTexture = NULL;
SDL_Texture *snakeHead = NULL;
SDL_Texture *snakeBody = NULL;
SDL_Texture *snakeTail = NULL;
SDL_Texture *foodTexture = NULL;
SDL_Texture *pauseIcon = NULL;
SDL_Texture *playIcon = NULL;
SDL_Texture *gameOver = NULL;
SDL_Texture *themeBackground = NULL;
SDL_Texture *menuBackground = NULL;
SDL_Texture *menuOptionPlay = NULL;
SDL_Texture *menuOptionHighestScore = NULL;
SDL_Texture *menuOptionHelp = NULL;
SDL_Texture *menuOptionExit = NULL;
SDL_Texture *restartIcon = NULL;
SDL_Texture *bonusFoodTexture = NULL;



// render-play-pause-button
SDL_Rect pauseRect = {SCREEN_WIDTH - 100, 20, 64, 64};
SDL_Rect playRect = {SCREEN_WIDTH - 100, 20, 64, 64};
// menu button rect
SDL_Rect menuPlayRect = {SCREEN_WIDTH / 2 - 70, 150, 140, 60};
SDL_Rect menuHighScoreRect = {SCREEN_WIDTH / 2 - 70, 218, 140, 60};
SDL_Rect menuHelpRect = {SCREEN_WIDTH / 2 - 70, 286, 140, 60};
SDL_Rect menuExitRect = {SCREEN_WIDTH / 2 - 70, 351, 140, 60};

TTF_Font *font = NULL;

SDL_Texture *scoreTexture = NULL;
SDL_Texture *scoreValueTexture = NULL;

// bgm and sound effect
Mix_Chunk *biteSound = NULL;
Mix_Chunk *mouseSound = NULL;
Mix_Chunk *clickSound = NULL;

Mix_Music *themeMusic = NULL;
Mix_Music *menuMusic = NULL;
Mix_Music *gameplayMusic = NULL;
Mix_Music *gameOverMusic = NULL;

bool isRunning;
bool isPaused = false;
bool isDead = false;

int score = 0;
int snakeSegmentSize = 24; // Size of each snake segment
Uint32 snakeSpeed = 200;   // Movement delay in milliseconds
Uint32 lastMoveTime = 0;   // Last time the snake moved

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};
Direction dir = RIGHT;
// section
enum Section
{
    THEME,
    MENU,
    GAMESCREEN
};
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

// load font form texture
// Function to render text and return it as an SDL_Texture
SDL_Texture *renderText(TTF_Font *font, const string &message, SDL_Color color, int &width, int &height)
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, message.c_str(), color);
    if (!textSurface)
    {
        cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << endl;
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    width = textSurface->w;  // Get the width of the rendered text
    height = textSurface->h; // Get the height of the rendered text
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

        // Ensure the food does not spawn on the snake
        // Ensure the bonus food does not spawn on the snake or within 100px of the boundary
        if (food.x < 50|| food.x >= SCREEN_WIDTH - 50 || food.y < 50 || food.y >= SCREEN_HEIGHT - 50)
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
        bonusFood.x = (rand() % (SCREEN_WIDTH / snakeSegmentSize)) * snakeSegmentSize;
        bonusFood.y = (rand() % (SCREEN_HEIGHT / snakeSegmentSize)) * snakeSegmentSize;

        // Ensure the bonus food does not spawn on the snake or within 100px of the boundary
        if (bonusFood.x < 50 || bonusFood.x >= SCREEN_WIDTH - 50|| bonusFood.y < 50 || bonusFood.y >= SCREEN_HEIGHT - 50)
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
    backgroundTexture = loadTexture("resource/g1.png", renderer);
    if (!backgroundTexture)
    {
        cout << "Error: Failed to load background texture" << endl;
        isRunning = false;
        return false;
    }

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

    // Load pause icon
    pauseIcon = loadTexture("resource/pause-icon.png", renderer);
    if (!pauseIcon)
    {
        cout << "Error: Failed to load pause texture" << endl;
        isRunning = false;
        return false;
    }
    // Load play icon
    playIcon = loadTexture("resource/play-icon.png", renderer);
    if (!playIcon)
    {
        cout << "Error: Failed to load play texture" << endl;
        isRunning = false;
        return false;
    }
    // Load play icon
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
    menuOptionPlay = loadTexture("resource/play.png", renderer);
    if (!menuOptionPlay)
    {
        cout << "Error: Failed to play texture" << endl;
        isRunning = false;
        return false;
    }
    menuOptionHighestScore = loadTexture("resource/highscore.png", renderer);
    if (!menuOptionHighestScore)
    {
        cout << "Error: Failed to high score texture" << endl;
        isRunning = false;
        return false;
    }
    menuOptionHelp = loadTexture("resource/help.png", renderer);
    if (!menuOptionHelp)
    {
        cout << "Error: Failed to help texture" << endl;
        isRunning = false;
        return false;
    }
    menuOptionExit = loadTexture("resource/exit.png", renderer);
    if (!menuOptionExit)
    {
        cout << "Error: Failed to exit texture" << endl;
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
    if (!biteSound ||  !mouseSound ||  !clickSound)
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

//---------------------------------------Event Handle----------------------------------------------

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
                Mix_PlayChannel(-1,mouseSound, 0);
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
                Mix_PlayChannel(-1,mouseSound, 0);
                currentSection = GAMESCREEN;
            }
            else if (x >= menuExitRect.x && x <= menuExitRect.x + menuExitRect.w &&
                     y >= menuExitRect.y && y <= menuExitRect.y + menuExitRect.h)
            {   
                Mix_PlayChannel(-1,mouseSound, 0);
                isRunning = false;
            }
        }
    }
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
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            int x, y;
            SDL_GetMouseState(&x, &y);

            if (!isDead)
            {
                if (!isPaused && x >= pauseRect.x && x <= pauseRect.x + pauseRect.w &&
                    y >= pauseRect.y && y <= pauseRect.y + pauseRect.h)
                {
                    isPaused = true;
                }
                else if (isPaused && x >= playRect.x && x <= playRect.x + playRect.w &&
                         y >= playRect.y && y <= playRect.y + playRect.h)
                {
                    isPaused = false;
                }
            }
        }
    }
}

bool checkCollision(const SDL_Rect &a, const SDL_Rect &b)
{
    return SDL_HasIntersection(&a, &b);
}

void update()
{
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastMoveTime < snakeSpeed)
    {
        return;
    }
    lastMoveTime = currentTime;

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
        Mix_PlayChannel(-1, biteSound, 0);

        // Place new food
        placeFood();
    }
    //cheak bonus food
    if (bonusFoodVisible)
    {
        SDL_Rect bonusFoodRect = {bonusFood.x, bonusFood.y, snakeSegmentSize, snakeSegmentSize};
        if (checkCollision(headRect, bonusFoodRect))
        {
            // Add new segment at the position of the last segment
            Segment newSegment = {snake.back().x, snake.back().y, snakeBody, 0.0};
            snake.insert(snake.end() - 1, newSegment);
            score += 50; // Bonus food gives more points
            Mix_PlayChannel(-1, biteSound, 0);

            // Hide bonus food
            bonusFoodVisible = false;
        }
    }

    if (!bonusFoodVisible &&(rand() % 100 < 5)) // 5% chance to spawn bonus food each update&& (rand() % 100 < 5)
    {
        placeBonusFood();
        bonusFoodVisible = true;
        bonusFoodAppearTime = currentTime;
    }
    if (bonusFoodVisible && currentTime - bonusFoodAppearTime > 5000) // Bonus food stays for 5 seconds
    {
        bonusFoodVisible = false;
    }

    // Check for self-collision
    for (size_t i = 1; i < snake.size(); ++i)
    {
        SDL_Rect segmentRect = {snake[i].x, snake[i].y, snakeSegmentSize, snakeSegmentSize};
        if (checkCollision(headRect, segmentRect))
        {
            isPaused = true;
            isDead = true;
            // Game over
        }
    }
    
    if(snake[0].x < 36|| snake[0].x >= SCREEN_WIDTH - 36 || snake[0].y < 36 || snake[0].y >= SCREEN_HEIGHT - 50)
    {
        isPaused = true;
        isDead = true;
    }
}

//-----------------------------------------------Render Part------------------------------------------

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
void menuRender()
{
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 210, 126, 26, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_RenderCopy(renderer, menuBackground, NULL, NULL);

    // Menu Button
    SDL_RenderCopy(renderer, menuOptionPlay, NULL, &menuPlayRect);
    SDL_RenderCopy(renderer, menuOptionHighestScore, NULL, &menuHighScoreRect);
    SDL_RenderCopy(renderer, menuOptionHelp, NULL, &menuHelpRect);
    SDL_RenderCopy(renderer, menuOptionExit, NULL, &menuExitRect);

    // Present the renderer
    SDL_RenderPresent(renderer);
}
void render()
{
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 210, 126, 26, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    // Render food
    SDL_Rect foodRect = {food.x, food.y, snakeSegmentSize, snakeSegmentSize};
    SDL_RenderCopy(renderer, food.texture, NULL, &foodRect);

    // Render bonus food if visible
    if (bonusFoodVisible)
    {
        SDL_Rect bonusFoodRect = {bonusFood.x, bonusFood.y, snakeSegmentSize, snakeSegmentSize};
        SDL_RenderCopy(renderer, bonusFood.texture, NULL, &bonusFoodRect);
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
    int scoreTextW, scoreTextH, scoreValueTextW, scoreValueTextH;
    scoreTexture = renderText(font, "Score:", textColor, scoreTextW, scoreTextH);
    scoreValueTexture = renderText(font, scoreValue, textColor, scoreValueTextW, scoreValueTextH);

    SDL_Rect scoreRect = {20, 20, scoreTextW, scoreTextH};
    SDL_Rect scoreValueRect = {20 + scoreTextW + 20, 20, scoreValueTextW, scoreValueTextH};
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    SDL_RenderCopy(renderer, scoreValueTexture, NULL, &scoreValueRect);

    // play-pause-icon
    if (!isDead)
    {
        if (isPaused)
        {
            SDL_RenderCopy(renderer, playIcon, NULL, &playRect);
        }
        else
        {
            SDL_RenderCopy(renderer, pauseIcon, NULL, &pauseRect);
        }
    }

    // game over screen
    if (isDead)
    {
        SDL_RenderCopy(renderer, gameOver, NULL, NULL);
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
    SDL_DestroyTexture(scoreTexture);
    SDL_DestroyTexture(scoreValueTexture);
    SDL_DestroyTexture(playIcon);
    SDL_DestroyTexture(pauseIcon);
    SDL_DestroyTexture(gameOver);
    SDL_DestroyTexture(themeBackground);
    SDL_DestroyTexture(menuBackground);
    SDL_DestroyTexture(menuOptionPlay);
    SDL_DestroyTexture(menuOptionHighestScore);
    SDL_DestroyTexture(menuOptionHelp);
    SDL_DestroyTexture(menuOptionExit);
    SDL_DestroyTexture(bonusFoodTexture);
    // clear music
    Mix_FreeChunk(biteSound);
    biteSound = NULL;
    Mix_FreeChunk(mouseSound);
    mouseSound= NULL;
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
    srand(static_cast<unsigned int>(time(0))); // Seed the random number generator

    if (!initializeWindow())
    {
        return -1;
    }

    isRunning = true;
    auto currenMusic = themeMusic;

// Start playing the initial music
Mix_PlayMusic(currenMusic, -1);

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

    case GAMESCREEN:
        
        handleEvents();
        if (!isPaused)
            update();
        render();
       if(!isDead){
         if (currenMusic != gameplayMusic)
        {
            currenMusic = gameplayMusic;
            Mix_PlayMusic(currenMusic, -1);
        }
       }else{
        if (currenMusic != gameOverMusic)
        {
            currenMusic = gameOverMusic;
            Mix_PlayMusic(gameOverMusic, -1);
        }
       }
        break;

    default:
        break;
    }
    
    // Cap the frame rate to about 60 FPS
    SDL_Delay(16);
}
    cleanUp();

    return 0;
}
