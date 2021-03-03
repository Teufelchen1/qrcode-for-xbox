#include <SDL.h>
#include <stdio.h>

#if defined(NXDK)
#include <hal/video.h>
#include <hal/debug.h>
#include <hal/xbox.h>
#include <windows.h>
#else
#include <unistd.h> // for sleep
#define debugPrint(...) printf(__VA_ARGS__) 
#define Sleep(...) sleep(__VA_ARGS__)
#endif

#include "qrcodegen.h"

static void printSDLErrorAndReboot(void)
{
    debugPrint("SDL_Error: %s\n", SDL_GetError());
    debugPrint("Rebooting in 5 seconds.\n");
    Sleep(5000);
#if defined(NXDK)
    XReboot();
#else
    exit(EXIT_FAILURE);
#endif
}

// Screen dimension constants
static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;
static const unsigned int COLOR_WHITE = 0x00ffffff; // ARGB
static const unsigned int COLOR_BLACK = 0x00000000;

// Creates a single QR Code, then prints it to the console.
int buildQRCode(uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX], const char *text)
{
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level
    
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    return qrcodegen_encodeText(
            text, tempBuffer, qrcode, errCorLvl,
            qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true
        );
}

void renderQRCode(SDL_Surface *surface, uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX])
{
    int size, padding; 
    SDL_Rect qrPixel;

    // Size is the number of 'little squares' in a QR-Symbol per row
    size = qrcodegen_getSize(qrcode);

    // The padding is the space left unused to the right+left of the symbol
    // (when it is centered on the screen)
    padding = (SCREEN_WIDTH - SCREEN_HEIGHT)/2;

    // By treating the screen as if it only has SCREEN_HEIGHT, 
    // we get the biggest square possible for a given screen.
    qrPixel.w = SCREEN_HEIGHT/size;
    qrPixel.h = SCREEN_HEIGHT/size;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            qrPixel.x = padding + x * qrPixel.w;
            qrPixel.y = y * qrPixel.h;
            SDL_FillRect(surface, &qrPixel, 
                (qrcodegen_getModule(qrcode, x, y) ? COLOR_WHITE : COLOR_BLACK)
            );
        }
    }
}



void displayQRCode(uint8_t qrcode[])
{
    int done = 0;
    SDL_Window *window;
    SDL_Event event;
    SDL_Surface *screenSurface;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_VideoInit(NULL) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL video.\n");
        printSDLErrorAndReboot();
    }

    window = SDL_CreateWindow("QR-Symbol",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if(window == NULL)
    {
        debugPrint( "Window could not be created!\n");
        SDL_VideoQuit();
        printSDLErrorAndReboot();
    }

    screenSurface = SDL_GetWindowSurface(window);
    if (!screenSurface) {
        SDL_VideoQuit();
        printSDLErrorAndReboot();
    }

    renderQRCode(screenSurface, qrcode);

    while (!done) {
        /* Check for events */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                done = 1;
                break;
            default:
                break;
            }
        }

        SDL_UpdateWindowSurface(window);
        Sleep(1);
    }

    SDL_VideoQuit();
}



// The main application program.
int main(void)
{
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];

#if defined(NXDK)
    XVideoSetMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, REFRESH_DEFAULT);
#endif

    if (!buildQRCode(qrcode, "https://www.youtube.com/watch?v=dQw4w9WgXcQ")) {
        debugPrint("Could not create QR-Code");
        Sleep(5);
#if defined(NXDK)
        XReboot();
#else
        return EXIT_FAILURE;
#endif
    }

    displayQRCode(qrcode);
    return EXIT_SUCCESS;
}