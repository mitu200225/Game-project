#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#include <cstdlib>
#include <ctime>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int GRID_SIZE = 20;
const int FRAME_RATE = 10;  // Adjust this value to control the frame rate

class Snake {
public:
    Snake(SDL_Renderer* renderer) : renderer(renderer) {
        // Initialize snake
        head = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
        body.push_back(head);

        // Initialize direction
        direction = {GRID_SIZE, 0};
    }

    void handleInput(SDL_Event& event) {
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (direction.y == 0) {
                        direction = {0, -GRID_SIZE};
                    }
                    break;
                case SDLK_DOWN:
                    if (direction.y == 0) {
                        direction = {0, GRID_SIZE};
                    }
                    break;
                case SDLK_LEFT:
                    if (direction.x == 0) {
                        direction = {-GRID_SIZE, 0};
                    }
                    break;
                case SDLK_RIGHT:
                    if (direction.x == 0) {
                        direction = {GRID_SIZE, 0};
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void move() {
        // Move the snake
        head.x += direction.x;
        head.y += direction.y;

        // Check for collisions with the screen borders
        if (head.x < 0 || head.x >= SCREEN_WIDTH || head.y < 0 || head.y >= SCREEN_HEIGHT) {
            reset();
            return;
        }

        // Check for collisions with itself
        for (const auto& segment : body) {
            if (head.x == segment.x && head.y == segment.y) {
                reset();
                return;
            }
        }

        // Move the body
        body.insert(body.begin(), head);

        // Check for collisions with food
        if (head.x == food.x && head.y == food.y) {
            score++;
            spawnFood();
        } else {
            body.pop_back();
        }
    }

    void render() {
        // Render the snake
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (const auto& segment : body) {
            SDL_Rect rect = {segment.x, segment.y, GRID_SIZE, GRID_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }

        // Render the food
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect foodRect = {food.x, food.y, GRID_SIZE, GRID_SIZE};
        SDL_RenderFillRect(renderer, &foodRect);
    }

    int getScore() const {
        return score;
    }

private:
    SDL_Renderer* renderer;
    SDL_Point head;
    std::vector<SDL_Point> body;
    SDL_Point direction;
    SDL_Point food;
    int score = 0;

    void spawnFood() {
        // Generate random coordinates for food
        food = {rand() % (SCREEN_WIDTH / GRID_SIZE) * GRID_SIZE,
                rand() % (SCREEN_HEIGHT / GRID_SIZE) * GRID_SIZE};

        // Ensure food doesn't spawn on the snake
        for (const auto& segment : body) {
            if (food.x == segment.x && food.y == segment.y) {
                spawnFood();
                return;
            }
        }
    }

    void reset() {
        // Reset snake and score
        head = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
        body.clear();
        body.push_back(head);
        direction = {GRID_SIZE, 0};
        score = 0;
        spawnFood();
    }
};

int main( int argc, char* argv[] ) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create SDL window and renderer
    SDL_Window* window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("mitu.otf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Seed random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Create Snake object
    Snake snake(renderer);

    // Main game loop
    bool quit = false;
    Uint32 lastTick = SDL_GetTicks();
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else {
                snake.handleInput(event);
            }
        }

        Uint32 currentTick = SDL_GetTicks();
        float deltaTime = (currentTick - lastTick) / 1000.0f;
        if (deltaTime >= 1.0f / FRAME_RATE) {
            snake.move();

            // Render the game
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            snake.render();

            // Render the score
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, ("Score: " + std::to_string(snake.getScore())).c_str(), textColor);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);

            SDL_Rect scoreRect;
            SDL_QueryTexture(textTexture, nullptr, nullptr, &scoreRect.w, &scoreRect.h);
            scoreRect.x = SCREEN_WIDTH - scoreRect.w - 10;
            scoreRect.y = 10;
            SDL_RenderCopy(renderer, textTexture, nullptr, &scoreRect);
            SDL_DestroyTexture(textTexture);

            SDL_RenderPresent(renderer);

            lastTick = currentTick;
        }
    }

    // Cleanup and exit
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}