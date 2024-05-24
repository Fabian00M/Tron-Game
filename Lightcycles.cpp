#include "GameEngine.h"
#include "Windows.h"
#include "Resource.h"
#include "BitMap.h"

// Global variables
GameEngine* game;
BitMap* bck, * blue0, * blue90, * blue180, * blue270; // Blue different directions
BitMap* orange0, * orange90, * orange180, * orange270; // Orange different directions
BitMap* blueCurrentBitmap, * orangeCurrentBitmap; // Current bitmaps to be drawn for blue and orange players

HDC offScreen = nullptr;
HBITMAP offScreenBitMap = nullptr;

const int MAX_SPEED = 4; // Define maximum speed for movement

int blueSpeedx = 0, orangeSpeedx = 0; // Speed in the x-direction for blue and orange players
int blueSpeedy = 0, orangeSpeedy = 0; // Speed in the y-direction for blue and orange players
int blueXPos = 0, orangeXPos = 0;       // Current x-positions for blue and orange players
int blueYPos = 0, orangeYPos = 0;       // Current y-positions for blue and orange players

std::vector<std::pair<int, int>> blueTrailPoints, orangeTrailPoints; // Trail points history

bool blueChangingDirection = false, orangeChangingDirection = false; // Flags to indicate if direction is currently changing

const int directionChangeDelay = 10; // Define the delay in frames
int blueDirectionChangeCounter = 0, orangeDirectionChangeCounter = 0; // Counters to track direction change delay 

// Function prototypes
BOOL GameInitialize(HINSTANCE currInstance);
void GameLoop();
void GameEnd();
void GameStart(HWND hwnd);
void GameActivate(HWND hwnd);
void GameDeactivate(HWND hwnd);
void GamePaint(HDC hdc);
void HandleKeys();
void MouseButtonDown(int x, int y, bool left);
void MouseButtonUp(int x, int y, bool left);
void MouseMove(int x, int y);
bool SpriteCollision(Sprite* hitter, Sprite* hittee);
void HandleCollision();

// Game initialization
BOOL GameInitialize(HINSTANCE currInstance) {
    // Create the game engine
    game = new GameEngine(currInstance, L"LightCycles", L"LightCycles", IDI_LightCycles, IDI_LightCycles_sm, 500, 400);
    if (game == NULL) {
        return FALSE;
    }
    // Set the frame rate
    game->setFrameRate(30);
    return TRUE;
}

// Main game loop
void GameLoop() {
    // Update sprite positions
    game->updateSprites();
    // Handle user input
    HandleKeys();
    // Check for collision with window edges
    HandleCollision();

    // Get window handle and device context
    HWND hwnd = game->getWnd();
    HDC hdc = GetDC(hwnd);

    // Create off-screen device context and bitmap if not already created
    if (offScreen == nullptr) {
        offScreen = CreateCompatibleDC(hdc);
        offScreenBitMap = CreateCompatibleBitmap(hdc, game->getWidth(), game->getHeight());
        SelectObject(offScreen, offScreenBitMap);
    }

    // Paint the game
    GamePaint(offScreen);

    // Copy the off-screen buffer to the window
    BitBlt(hdc, 0, 0, game->getWidth(), game->getHeight(), offScreen, 0, 0, SRCCOPY);

    // Release device context
    ReleaseDC(hwnd, hdc);

    // Increment direction change counters
    if (blueChangingDirection) {
        blueDirectionChangeCounter++;
        if (blueDirectionChangeCounter >= directionChangeDelay) {
            blueDirectionChangeCounter = 0;
            blueChangingDirection = false; // Reset blueChangingDirection flag after delay
        }
    }
    if (orangeChangingDirection) {
        orangeDirectionChangeCounter++;
        if (orangeDirectionChangeCounter >= directionChangeDelay) {
            orangeDirectionChangeCounter = 0;
            orangeChangingDirection = false; // Reset orangeChangingDirection flag after delay
        }
    }
}

// Game cleanup
void GameEnd() {
    // Delete off-screen bitmap and device context if they exist
    if (offScreenBitMap != nullptr) {
        DeleteObject(offScreenBitMap);
    }
    if (offScreen != nullptr) {
        DeleteDC(offScreen);
    }
    // Delete background and bitmap objects
    delete bck;
    delete blue0;
    delete blue90;
    delete blue180;
    delete blue270;
    delete orange0;
    delete orange90;
    delete orange180;
    delete orange270;
    // Delete game engine
    delete game;

    PostQuitMessage(0);
}

// Start/restart the game
void GameStart(HWND hwnd) {
    // Initialize random number generator
    srand(GetTickCount());
    // Get window device context
    HDC hdc = GetDC(hwnd);
    // Load background and bitmap images
    bck = new BitMap(hdc, L"Res/Background.bmp");
    blue0 = new BitMap(hdc, L"Res/CycleBlue_0.bmp");
    blue90 = new BitMap(hdc, L"Res/CycleBlue_90.bmp");
    blue180 = new BitMap(hdc, L"Res/CycleBlue_180.bmp");
    blue270 = new BitMap(hdc, L"Res/CycleBlue_270.bmp");
    orange0 = new BitMap(hdc, L"Res/CycleOrange_0.bmp");
    orange90 = new BitMap(hdc, L"Res/CycleOrange_90.bmp");
    orange180 = new BitMap(hdc, L"Res/CycleOrange_180.bmp");
    orange270 = new BitMap(hdc, L"Res/CycleOrange_270.bmp");
    // Set the initial bitmaps to blue0 and orange0
    blueCurrentBitmap = blue0;
    orangeCurrentBitmap = orange180;
    // Release device context
    ReleaseDC(hwnd, hdc);

    // Set initial positions of the bitmaps to custom positions
    blueXPos = 250;    // Blue starting x-position
    blueYPos = 350;    // Blue starting y-position
    orangeXPos = 250;  // Orange starting x-position
    orangeYPos = 25;  // Orange starting y-position

    // Reset speeds to zero
    blueSpeedx = 0;
    orangeSpeedx = 0;
    blueSpeedy = 0;
    orangeSpeedy = 0;

    // Clear trailPoints vectors
    blueTrailPoints.clear();
    orangeTrailPoints.clear();
}

// Game activation handler
void GameActivate(HWND hwnd) {
    // Blank
}

// Game deactivation handler
void GameDeactivate(HWND hwnd) {
    // Blank
}

// Game painting function
void GamePaint(HDC hdc) {
    if (bck != nullptr) {
        // Draw the background
        bck->draw(hdc, 0, 0);
    }

    if (blueCurrentBitmap != nullptr) {
        // Draw the current blue bitmap at its current position
        blueCurrentBitmap->draw(hdc, blueXPos, blueYPos);

        // Draw blue trail
        HPEN bluePen = CreatePen(PS_SOLID, 0.5, RGB(0, 0, 255)); // Blue color
        HPEN hOldPen = (HPEN)SelectObject(hdc, bluePen);
        for (size_t i = 1; i < blueTrailPoints.size(); ++i) {
            MoveToEx(hdc, blueTrailPoints[i - 1].first, blueTrailPoints[i - 1].second, nullptr);
            LineTo(hdc, blueTrailPoints[i].first, blueTrailPoints[i].second);
        }
        SelectObject(hdc, hOldPen);
        DeleteObject(bluePen);

        // Add current position to blueTrailPoints
        blueTrailPoints.push_back(std::make_pair(blueXPos + blueCurrentBitmap->getWidth() / 2, blueYPos + blueCurrentBitmap->getHeight() / 2));
    }

    if (orangeCurrentBitmap != nullptr) {
        // Draw the current orange bitmap at its current position
        orangeCurrentBitmap->draw(hdc, orangeXPos, orangeYPos);

        // Draw orange trail
        HPEN orangePen = CreatePen(PS_SOLID, 0.5, RGB(255, 165, 0)); // Orange color
        HPEN hOldPen = (HPEN)SelectObject(hdc, orangePen);
        for (size_t i = 1; i < orangeTrailPoints.size(); ++i) {
            MoveToEx(hdc, orangeTrailPoints[i - 1].first, orangeTrailPoints[i - 1].second, nullptr);
            LineTo(hdc, orangeTrailPoints[i].first, orangeTrailPoints[i].second);
        }
        SelectObject(hdc, hOldPen);
        DeleteObject(orangePen);

        // Add current position to orangeTrailPoints
        orangeTrailPoints.push_back(std::make_pair(orangeXPos + orangeCurrentBitmap->getWidth() / 2, orangeYPos + orangeCurrentBitmap->getHeight() / 2));
    }
}

// Handle keyboard input
void HandleKeys() {
    // Reset speed in one direction when the corresponding arrow key is pressed
    if (!blueChangingDirection) {
        if (GetAsyncKeyState(VK_UP) < 0 && blueSpeedy >= 0) {
            // Move up for blue player
            blueSpeedy = max(-MAX_SPEED, --blueSpeedy);
            // Reset horizontal speed
            blueSpeedx = 0;
            // Set the current bitmap to blue0
            blueCurrentBitmap = blue0;
            blueChangingDirection = true;
        }
        else if (GetAsyncKeyState(VK_DOWN) < 0 && blueSpeedy <= 0) {
            // Move down for blue player
            blueSpeedy = min(MAX_SPEED, ++blueSpeedy);
            // Reset horizontal speed
            blueSpeedx = 0;
            // Set the current bitmap to blue180
            blueCurrentBitmap = blue180;
            blueChangingDirection = true;
        }
        else if (GetAsyncKeyState(VK_LEFT) < 0 && blueSpeedx >= 0) {
            // Move left for blue player
            blueSpeedx = max(-MAX_SPEED, --blueSpeedx);
            // Reset vertical speed
            blueSpeedy = 0;
            // Set the current bitmap to blue270
            blueCurrentBitmap = blue270;
            blueChangingDirection = true;
        }
        else if (GetAsyncKeyState(VK_RIGHT) < 0 && blueSpeedx <= 0) {
            // Move right for blue player
            blueSpeedx = min(MAX_SPEED, ++blueSpeedx);
            // Reset vertical speed
            blueSpeedy = 0;
            // Set the current bitmap to blue90
            blueCurrentBitmap = blue90;
            blueChangingDirection = true;
        }
    }

    if (!orangeChangingDirection) {
        if ((GetAsyncKeyState(0x57) < 0 || GetAsyncKeyState(0x57) == -32767) && orangeSpeedy >= 0) { // W key
            // Move up for orange player
            orangeSpeedy = max(-MAX_SPEED, --orangeSpeedy);
            // Reset horizontal speed
            orangeSpeedx = 0;
            // Set the current bitmap to orange0
            orangeCurrentBitmap = orange0;
            orangeChangingDirection = true;
        }
        else if ((GetAsyncKeyState(0x53) < 0 || GetAsyncKeyState(0x53) == -32767) && orangeSpeedy <= 0) { // S key
            // Move down for orange player
            orangeSpeedy = min(MAX_SPEED, ++orangeSpeedy);
            // Reset horizontal speed
            orangeSpeedx = 0;
            // Set the current bitmap to orange180
            orangeCurrentBitmap = orange180;
            orangeChangingDirection = true;
        }
        else if ((GetAsyncKeyState(0x41) < 0 || GetAsyncKeyState(0x41) == -32767) && orangeSpeedx >= 0) { // A key
            // Move left for orange player
            orangeSpeedx = max(-MAX_SPEED, --orangeSpeedx);
            // Reset vertical speed
            orangeSpeedy = 0;
            // Set the current bitmap to orange270
            orangeCurrentBitmap = orange270;
            orangeChangingDirection = true;
        }
        else if ((GetAsyncKeyState(0x44) < 0 || GetAsyncKeyState(0x44) == -32767) && orangeSpeedx <= 0) { // D key
            // Move right for orange player
            orangeSpeedx = min(MAX_SPEED, ++orangeSpeedx);
            // Reset vertical speed
            orangeSpeedy = 0;
            // Set the current bitmap to orange90
            orangeCurrentBitmap = orange90;
            orangeChangingDirection = true;
        }
    }

    // Update the positions based on current speeds
    blueXPos = min(game->getWidth() - blueCurrentBitmap->getWidth(), max(0, blueXPos + blueSpeedx));
    blueYPos = min(game->getHeight() - blueCurrentBitmap->getHeight(), max(0, blueYPos + blueSpeedy));
    orangeXPos = min(game->getWidth() - orangeCurrentBitmap->getWidth(), max(0, orangeXPos + orangeSpeedx));
    orangeYPos = min(game->getHeight() - orangeCurrentBitmap->getHeight(), max(0, orangeYPos + orangeSpeedy));
}

// Handle mouse button down event
void MouseButtonDown(int x, int y, bool left) {
}

// Handle mouse button up event
void MouseButtonUp(int x, int y, bool left) {
}

// Handle mouse move event
void MouseMove(int x, int y) {
}

// Handle sprite collision
bool SpriteCollision(Sprite* hitter, Sprite* hittee) {
    return false;
}

// Handle collision with window edges and trails
// Handle collision with window edges and trails
void HandleCollision() {
    // Check if the bitmaps touch the edges of the window
    if (blueXPos <= 0 || blueXPos >= game->getWidth() - blueCurrentBitmap->getWidth() ||
        blueYPos <= 0 || blueYPos >= game->getHeight() - blueCurrentBitmap->getHeight()) {
        int result = MessageBox(game->getWnd(), L"Orange player Wins!\nDo you want to restart the game?", L"Game Over", MB_YESNO | MB_ICONQUESTION);
        if (result == IDYES) {
            GameStart(game->getWnd());
        }
        else {
            GameEnd();
        }
    }

    if (orangeXPos <= 0 || orangeXPos >= game->getWidth() - orangeCurrentBitmap->getWidth() ||
        orangeYPos <= 0 || orangeYPos >= game->getHeight() - orangeCurrentBitmap->getHeight()) {
        int result = MessageBox(game->getWnd(), L"Blue player Wins!\nDo you want to restart the game?", L"Game Over", MB_YESNO | MB_ICONQUESTION);
        if (result == IDYES) {
            GameStart(game->getWnd());
        }
        else {
            GameEnd();
        }
    }

    // Check if the blue player collides with the orange trail or its own trail
    for (size_t i = 1; i < orangeTrailPoints.size(); ++i) {
        int x1 = orangeTrailPoints[i - 1].first, y1 = orangeTrailPoints[i - 1].second;
        int x2 = orangeTrailPoints[i].first, y2 = orangeTrailPoints[i].second;
        if ((blueXPos + blueCurrentBitmap->getWidth() / 2 >= min(x1, x2) && blueXPos + blueCurrentBitmap->getWidth() / 2 <= max(x1, x2)) &&
            (blueYPos + blueCurrentBitmap->getHeight() / 2 >= min(y1, y2) && blueYPos + blueCurrentBitmap->getHeight() / 2 <= max(y1, y2))) {
            int result = MessageBox(game->getWnd(), L"Orange player Wins!\nDo you want to restart the game?", L"Game Over", MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES) {
                GameStart(game->getWnd());
            }
            else {
                GameEnd();
            }
            return;
        }
    }

    // Check if the orange player collides with the blue trail or its own trail
    for (size_t i = 1; i < blueTrailPoints.size(); ++i) {
        int x1 = blueTrailPoints[i - 1].first, y1 = blueTrailPoints[i - 1].second;
        int x2 = blueTrailPoints[i].first, y2 = blueTrailPoints[i].second;
        if ((orangeXPos + orangeCurrentBitmap->getWidth() / 2 >= min(x1, x2) && orangeXPos + orangeCurrentBitmap->getWidth() / 2 <= max(x1, x2)) &&
            (orangeYPos + orangeCurrentBitmap->getHeight() / 2 >= min(y1, y2) && orangeYPos + orangeCurrentBitmap->getHeight() / 2 <= max(y1, y2))) {
            int result = MessageBox(game->getWnd(), L"Blue player Wins!\nDo you want to restart the game?", L"Game Over", MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES) {
                GameStart(game->getWnd());
            }
            else {
                GameEnd();
            }
            return;
        }
    }
}
