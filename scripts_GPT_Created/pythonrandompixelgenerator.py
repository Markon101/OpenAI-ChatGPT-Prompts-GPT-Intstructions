import pygame
import random
import concurrent.futures
import asyncio

# Function to draw a pixel with random color
def draw_pixel(screen, x, y, pixel_size):
    color = (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
    pygame.draw.rect(screen, color, (x, y, pixel_size, pixel_size))

# Async function to update the screen concurrently
async def update_screen(screen, width, height, pixel_size, executor):
    while True:
        # Schedule the draw_pixel tasks and run them concurrently
        futures = [
            executor.submit(draw_pixel, screen, x, y, pixel_size)
            for x in range(0, width, pixel_size)
            for y in range(0, height, pixel_size)
        ]
        concurrent.futures.wait(futures)
        
        # Update the full display Surface to the screen
        pygame.display.flip()
        
        # Small delay to make the loop responsive
        await asyncio.sleep(0.01)

def main():
    # Screen dimensions and pixel size (modifiable by the user)
    width, height, pixel_size = 800, 600, 10

    # Initialize Pygame
    pygame.init()
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption("Random Pixel Screen")

    # Create a ThreadPoolExecutor
    executor = concurrent.futures.ThreadPoolExecutor()

    # Async loop
    loop = asyncio.get_event_loop()
    try:
        asyncio.ensure_future(update_screen(screen, width, height, pixel_size, executor))
        loop.run_forever()
    except KeyboardInterrupt:
        pass
    finally:
        executor.shutdown()
        pygame.quit()

if __name__ == "__main__":
    main()

