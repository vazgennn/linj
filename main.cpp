#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <tlhelp32.h>

// Color definitions
#define COLOR_BACKGROUND RGB(20, 20, 20)
#define COLOR_HEADER RGB(180, 30, 30)
#define COLOR_TEXT_WHITE RGB(255, 255, 255)
#define COLOR_TEXT_RED RGB(255, 50, 50)
#define COLOR_PANEL RGB(30, 30, 30)

// Global variables
bool showGUI = false;
bool fullbrightEnabled = false;
bool processDetected = false;
std::string detectedProcess = "";

struct Category {
    std::string name;
    std::vector<std::pair<std::string, bool>> features;
    int x, y, width, height;
    bool dragged;
    POINT dragOffset;
};

std::vector<Category> categories;

// Function declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Render(HDC hdc, int width, int height);
void CheckMinecraftProcess();
void ToggleFullbright();
void InitializeCategories();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize categories
    InitializeCategories();
    
    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(COLOR_BACKGROUND);
    wc.lpszClassName = "MinecraftClientOverlay";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClassEx(&wc);
    
    // Create transparent overlay window
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        "MinecraftClientOverlay",
        "Minecraft Client",
        WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );
    
    SetLayeredWindowAttributes(hwnd, COLOR_BACKGROUND, 255, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

void InitializeCategories() {
    categories.clear();
    
    Category visual;
    visual.name = "Visual";
    visual.features = {{"Fullbright", false}, {"ESP", false}, {"Nametags", false}};
    visual.x = 50;
    visual.y = 50;
    visual.width = 200;
    visual.height = 300;
    visual.dragged = false;
    categories.push_back(visual);
    
    Category combat;
    combat.name = "Combat";
    combat.features = {{"KillAura", false}, {"AutoClicker", false}, {"Reach", false}};
    combat.x = 300;
    combat.y = 50;
    combat.width = 200;
    combat.height = 300;
    combat.dragged = false;
    categories.push_back(combat);
    
    Category movement;
    movement.name = "Movement";
    movement.features = {{"Fly", false}, {"Speed", false}, {"NoFall", false}};
    movement.x = 550;
    movement.y = 50;
    movement.width = 200;
    movement.height = 300;
    movement.dragged = false;
    categories.push_back(movement);
    
    Category player;
    player.name = "Player";
    player.features = {{"NoRotate", false}, {"AntiAFK", false}, {"FastPlace", false}};
    player.x = 50;
    player.y = 400;
    player.width = 200;
    player.height = 300;
    player.dragged = false;
    categories.push_back(player);
    
    Category world;
    world.name = "World";
    world.features = {{"Nuker", false}, {"Scaffold", false}, {"Tower", false}};
    world.x = 300;
    world.y = 400;
    world.width = 200;
    world.height = 300;
    world.dragged = false;
    categories.push_back(world);
    
    Category misc;
    misc.name = "Misc";
    misc.features = {{"Timer", false}, {"Freecam", false}, {"NoSlowdown", false}};
    misc.x = 550;
    misc.y = 400;
    misc.width = 200;
    misc.height = 300;
    misc.dragged = false;
    categories.push_back(misc);
}

void CheckMinecraftProcess() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    processDetected = false;
    detectedProcess = "";
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            std::wstring wname(pe32.szExeFile);
            std::string name(wname.begin(), wname.end());
            
            // Convert to lowercase for comparison
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            
            if (name == "javaw.exe" || name == "minecraft.windows.exe" || name == "minecraft.exe") {
                processDetected = true;
                detectedProcess = name;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
}

void ToggleFullbright() {
    fullbrightEnabled = !fullbrightEnabled;
    // In a real implementation, you would modify Minecraft's gamma setting here
    // This is just a placeholder for the toggle functionality
}

void RenderCategory(HDC hdc, Category& cat, int mouseX, int mouseY) {
    int headerHeight = 25;
    int featureHeight = 20;
    int padding = 5;
    
    // Draw panel background
    RECT panelRect = {cat.x, cat.y, cat.x + cat.width, cat.y + cat.height};
    HBRUSH panelBrush = CreateSolidBrush(COLOR_PANEL);
    FillRect(hdc, &panelRect, panelBrush);
    DeleteObject(panelBrush);
    
    // Draw header background
    RECT headerRect = {cat.x, cat.y, cat.x + cat.width, cat.y + headerHeight};
    HBRUSH headerBrush = CreateSolidBrush(COLOR_HEADER);
    FillRect(hdc, &headerRect, headerBrush);
    DeleteObject(headerBrush);
    
    // Draw header text with feature count
    std::string headerText = cat.name + " [" + std::to_string(cat.features.size()) + "]";
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_TEXT_WHITE);
    DrawTextA(hdc, headerText.c_str(), -1, &headerRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Draw features
    int yPos = cat.y + headerHeight + padding;
    for (auto& feature : cat.features) {
        RECT featureRect = {cat.x + padding, yPos, cat.x + cat.width - padding, yPos + featureHeight};
        
        // Set text color based on active state
        if (feature.second) {
            SetTextColor(hdc, COLOR_TEXT_RED);
        } else {
            SetTextColor(hdc, COLOR_TEXT_WHITE);
        }
        
        DrawTextA(hdc, feature.first.c_str(), -1, &featureRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        yPos += featureHeight + 2;
    }
    
    // Draw border
    HPEN borderPen = CreatePen(PS_SOLID, 1, COLOR_HEADER);
    SelectObject(hdc, borderPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, cat.x, cat.y, cat.x + cat.width, cat.y + cat.height);
    DeleteObject(borderPen);
}

void Render(HDC hdc, int width, int height) {
    // Clear background
    RECT bgRect = {0, 0, width, height};
    HBRUSH bgBrush = CreateSolidBrush(COLOR_BACKGROUND);
    FillRect(hdc, &bgRect, bgBrush);
    DeleteObject(bgBrush);
    
    if (!showGUI) return;
    
    // Check for Minecraft process
    CheckMinecraftProcess();
    
    // Draw process status
    std::string statusText = processDetected ? 
        "Minecraft Detected: " + detectedProcess : 
        "Minecraft Not Running";
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, processDetected ? COLOR_TEXT_RED : COLOR_TEXT_WHITE);
    RECT statusRect = {10, 10, width - 10, 30};
    DrawTextA(hdc, statusText.c_str(), -1, &statusRect, DT_LEFT | DT_SINGLELINE);
    
    // Draw all categories
    for (auto& cat : categories) {
        RenderCategory(hdc, cat, 0, 0);
    }
    
    // Draw Fullbright status
    std::string fbStatus = fullbrightEnabled ? "Fullbright: ON" : "Fullbright: OFF";
    SetTextColor(hdc, fullbrightEnabled ? COLOR_TEXT_RED : COLOR_TEXT_WHITE);
    RECT fbRect = {10, height - 30, width - 10, height - 10};
    DrawTextA(hdc, fbStatus.c_str(), -1, &fbRect, DT_RIGHT | DT_SINGLELINE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND doubleBufferHWND = NULL;
    static HDC memoryDC = NULL;
    static HBITMAP memoryBitmap = NULL;
    static int bmpWidth = 0, bmpHeight = 0;
    
    switch (msg) {
        case WM_CREATE: {
            // Set timer for periodic updates
            SetTimer(hwnd, 1, 16, NULL); // ~60 FPS
            return 0;
        }
        
        case WM_TIMER: {
            if (wParam == 1) {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            int width = ps.rcPaint.right - ps.rcPaint.left;
            int height = ps.rcPaint.bottom - ps.rcPaint.top;
            
            // Create double buffer if needed or if size changed
            if (doubleBufferHWND == NULL || bmpWidth != width || bmpHeight != height) {
                if (memoryDC) {
                    DeleteDC(memoryDC);
                    DeleteObject(memoryBitmap);
                }
                
                doubleBufferHWND = hwnd;
                bmpWidth = width;
                bmpHeight = height;
                
                memoryDC = CreateCompatibleDC(hdc);
                memoryBitmap = CreateCompatibleBitmap(hdc, width, height);
                SelectObject(memoryDC, memoryBitmap);
            }
            
            // Render to memory DC
            Render(memoryDC, width, height);
            
            // Copy to screen
            BitBlt(hdc, 0, 0, width, height, memoryDC, 0, 0, SRCCOPY);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_KEYDOWN: {
            if (wParam == VK_INSERT) {
                showGUI = !showGUI;
            }
            if (wParam == 'F' && GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                ToggleFullbright();
            }
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            int mouseX = GET_X_LPARAM(lParam);
            int mouseY = GET_Y_LPARAM(lParam);
            
            // Check if clicking on category headers
            for (auto& cat : categories) {
                if (mouseX >= cat.x && mouseX <= cat.x + cat.width &&
                    mouseY >= cat.y && mouseY <= cat.y + 25) {
                    cat.dragged = true;
                    cat.dragOffset.x = mouseX - cat.x;
                    cat.dragOffset.y = mouseY - cat.y;
                    break;
                }
            }
            return 0;
        }
        
        case WM_LBUTTONUP: {
            for (auto& cat : categories) {
                cat.dragged = false;
            }
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            int mouseX = GET_X_LPARAM(lParam);
            int mouseY = GET_Y_LPARAM(lParam);
            
            for (auto& cat : categories) {
                if (cat.dragged) {
                    cat.x = mouseX - cat.dragOffset.x;
                    cat.y = mouseY - cat.dragOffset.y;
                }
            }
            return 0;
        }
        
        case WM_RBUTTONDOWN: {
            int mouseX = GET_X_LPARAM(lParam);
            int mouseY = GET_Y_LPARAM(lParam);
            
            // Toggle features on right click
            for (auto& cat : categories) {
                int headerHeight = 25;
                int featureHeight = 20;
                int padding = 5;
                int yPos = cat.y + headerHeight + padding;
                
                if (mouseX >= cat.x && mouseX <= cat.x + cat.width &&
                    mouseY >= cat.y + headerHeight && mouseY <= cat.y + cat.height) {
                    
                    int index = (mouseY - (cat.y + headerHeight + padding)) / (featureHeight + 2);
                    if (index >= 0 && index < cat.features.size()) {
                        cat.features[index].second = !cat.features[index].second;
                        
                        // Special handling for Fullbright
                        if (cat.name == "Visual" && cat.features[index].first == "Fullbright") {
                            ToggleFullbright();
                        }
                        break;
                    }
                }
            }
            return 0;
        }
        
        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            if (memoryDC) {
                DeleteDC(memoryDC);
                DeleteObject(memoryBitmap);
            }
            PostQuitMessage(0);
            return 0;
        }
        
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
