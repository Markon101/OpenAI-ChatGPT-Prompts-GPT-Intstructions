#include <SDL2/SDL.h>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <queue>

const int WIDTH = 1280;
const int HEIGHT = 1280;
const int PIXEL_SIZE = 4;
const int NUM_THREADS = 8;

struct PixelInfo {
    int x, y;
    SDL_Color color;
};

std::mutex queueMutex;
std::queue<PixelInfo> pixelQueue;

void generateRandomPixels(int threadId) {
    int xStart = (WIDTH / NUM_THREADS) * threadId;
    int xEnd = (WIDTH / NUM_THREADS) * (threadId + 1);

    for (int x = xStart; x < xEnd; x += PIXEL_SIZE) {
        for (int y = 0; y < HEIGHT; y += PIXEL_SIZE) {
            PixelInfo p;
            p.x = x;
            p.y = y;
            p.color = { static_cast<Uint8>(rand() % 256),
                        static_cast<Uint8>(rand() % 256),
                        static_cast<Uint8>(rand() % 256),
                        255 };

            std::lock_guard<std::mutex> guard(queueMutex);
            pixelQueue.push(p);
        }
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Random Pixel Screen",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    srand(static_cast<unsigned>(time(nullptr)));

    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.push_back(std::thread(generateRandomPixels, i));
        }

        for (auto& t : threads) {
            t.join();
        }

        while (!pixelQueue.empty()) {
            queueMutex.lock();
            PixelInfo p = pixelQueue.front();
            pixelQueue.pop();
            queueMutex.unlock();

            SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, p.color.a);
            SDL_Rect rect = {p.x, p.y, PIXEL_SIZE, PIXEL_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

