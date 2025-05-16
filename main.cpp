#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <cmath>
#include <chrono>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

// Cross-platform keyboard input handling
class KeyboardInput {
private:
#ifndef _WIN32
    struct termios oldt, newt;
    int oldf;
    
    void init() {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    }
    
    void cleanup() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
    }
#endif

public:
    KeyboardInput() {
#ifndef _WIN32
        init();
#endif
    }
    
    ~KeyboardInput() {
#ifndef _WIN32
        cleanup();
#endif
    }
    
    int getKey() {
#ifdef _WIN32
        if (_kbhit()) {
            return _getch();
        }
        return 0;
#else
        int ch = getchar();
        if (ch == EOF) {
            return 0;
        }
        return ch;
#endif
    }
    
    bool kbhit() {
#ifdef _WIN32
        return _kbhit() != 0;
#else
        int byteswaiting;
        ioctl(STDIN_FILENO, FIONREAD, &byteswaiting);
        return byteswaiting > 0;
#endif
    }
};

// Thread-safe console output
class ConsoleWriter {
private:
#ifdef _WIN32
    HANDLE console;
    COORD cursorPos;
#endif
    
public:
    ConsoleWriter() {
#ifdef _WIN32
        console = GetStdHandle(STD_OUTPUT_HANDLE);
        cursorPos.X = 0;
        cursorPos.Y = 0;
#endif
    }
    
    void clearScreen() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD count;
        DWORD cellCount;
        
        if (GetConsoleScreenBufferInfo(console, &csbi)) {
            cellCount = csbi.dwSize.X * csbi.dwSize.Y;
            COORD homeCoords = {0, 0};
            FillConsoleOutputCharacter(console, ' ', cellCount, homeCoords, &count);
            FillConsoleOutputAttribute(console, csbi.wAttributes, cellCount, homeCoords, &count);
            SetConsoleCursorPosition(console, homeCoords);
        }
#else
        std::cout << "\033[2J\033[1;1H" << std::flush;
#endif
    }
    
    void setCursorPosition(int x, int y) {
#ifdef _WIN32
        cursorPos.X = static_cast<SHORT>(x);
        cursorPos.Y = static_cast<SHORT>(y);
        SetConsoleCursorPosition(console, cursorPos);
#else
        std::cout << "\033[" << y + 1 << ";" << x + 1 << "H" << std::flush;
#endif
    }
    
    void setTextColor(int color) {
#ifdef _WIN32
        SetConsoleTextAttribute(console, color);
#else
        // ANSI color codes for Linux/Unix
        switch (color) {
            case 7:  std::cout << "\033[0m"; break;     // White (default)
            case 10: std::cout << "\033[32m"; break;    // Green
            case 12: std::cout << "\033[31m"; break;    // Red
            case 6: std::cout << "\033[33m"; break;     // Yellow
            case 11: std::cout << "\033[36m"; break;    // Cyan
        }
#endif
    }
    
    void hideCursor() {
#ifdef _WIN32
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(console, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(console, &cursorInfo);
#else
        std::cout << "\033[?25l" << std::flush;
#endif
    }
    
    void showCursor() {
#ifdef _WIN32
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(console, &cursorInfo);
        cursorInfo.bVisible = TRUE;
        SetConsoleCursorInfo(console, &cursorInfo);
#else
        std::cout << "\033[?25h" << std::flush;
#endif
    }
};

// Main class for the CPU heater
class CPUHeater {
private:
    std::vector<std::thread> threads;
    std::atomic<bool> running;
    std::atomic<int> intensity;
    int numCores;
    ConsoleWriter console;
    KeyboardInput keyboard;
    
    // Define colors for different platforms
#ifdef _WIN32
    const int COLOR_DEFAULT = 7;   // White
    const int COLOR_HOT = 12;      // Red
    const int COLOR_WARM = 6;     // Yellow
    const int COLOR_COOL = 10;     // Green
    const int COLOR_TITLE = 11;    // Cyan
#else
    const int COLOR_DEFAULT = 7;
    const int COLOR_HOT = 12;
    const int COLOR_WARM = 6;
    const int COLOR_COOL = 10;
    const int COLOR_TITLE = 11;
#endif
    
    void workerThread(int threadId) {
        long double a = 1.0, b = 1.0, c = 1.0, d = 1.0, e = 1.0;
        
        // Each cycle will take this much time (in milliseconds)
        const int CYCLE_TIME_MS = 100;
        
        while (running) {
            auto cycleStart = std::chrono::high_resolution_clock::now();
            
            // Calculate how much of the cycle time should be spent calculating
            // based on intensity percentage
            int calculationTimeMs = (intensity * CYCLE_TIME_MS) / 100;
            
            // Perform calculations until the allocated calculation time is used
            auto calculationEnd = cycleStart + std::chrono::milliseconds(calculationTimeMs);
            
            while (std::chrono::high_resolution_clock::now() < calculationEnd && running) {
                // Do our intensive calculations
                a = std::sin(a) * std::cos(b) * std::tan(c) * std::sqrt(std::abs(d)) * std::log(1 + std::abs(e));
                b = std::cos(b) * std::tan(c) * std::sqrt(std::abs(d)) * std::log(1 + std::abs(e)) * std::sin(a);
                c = std::tan(c) * std::sqrt(std::abs(d)) * std::log(1 + std::abs(e)) * std::sin(a) * std::cos(b);
                d = std::sqrt(std::abs(d)) * std::log(1 + std::abs(e)) * std::sin(a) * std::cos(b) * std::tan(c);
                e = std::log(1 + std::abs(e)) * std::sin(a) * std::cos(b) * std::tan(c) * std::sqrt(std::abs(d));
                
                // Additional exponential, logarithmic and trigonometric operations
                a = std::pow(std::abs(a) + 1.0, 1.5) * std::exp(std::sin(b));
                b = std::log10(std::abs(b) + 1.1) * std::asin(std::cos(c));
                c = std::sinh(std::abs(c) * 0.1) * std::acos(std::sin(d));
                d = std::cosh(std::abs(d) * 0.1) * std::atan(std::cos(e));
                e = std::tanh(std::abs(e) * 0.1) * std::atan2(a, std::abs(b) + 0.1);
            }
            
            // Sleep for the remainder of the cycle time
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto cycleEnd = cycleStart + std::chrono::milliseconds(CYCLE_TIME_MS);
            
            if (currentTime < cycleEnd) {
                std::this_thread::sleep_for(cycleEnd - currentTime);
            }
        }
    }
    
    void displayThread() {
        int lastIntensity = 0;
        
        while (running) {
            console.setCursorPosition(0, 5);
            console.setTextColor(intensity > 75 ? COLOR_HOT : 
                               intensity > 40 ? COLOR_WARM : COLOR_COOL);
            
            std::cout << "Current Calculation Intensity: ";
            // Draw a progress bar
            std::cout << "[";
            for (int i = 0; i < 50; ++i) {
                if (i < intensity / 2) {
                    std::cout << "#";
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "] " << intensity << "% ";
            
            if (intensity > 75) {
                std::cout << "MAXIMUM LOAD    " << std::endl;
            } else if (intensity > 40) {
                std::cout << "HIGH LOAD       " << std::endl;
            } else {
                std::cout << "MODERATE LOAD   " << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void handleInput() {
        while (running) {
            if (keyboard.kbhit()) {
                int key = keyboard.getKey();
                
                // Up and down arrows have different codes on Windows and Unix
#ifdef _WIN32
                if (key == 224 || key == 0) { // Special key prefix on Windows
                    key = keyboard.getKey();
                    if (key == 72) { // Up arrow
                        if (intensity < 100) intensity++;
                    } else if (key == 80) { // Down arrow
                        if (intensity > 0) intensity--;
                    }
                }
#else
                if (key == 27) { // ESC prefix for arrow keys in Unix
                    if (keyboard.getKey() == 91) { // [ prefix
                        key = keyboard.getKey();
                        if (key == 65) { // Up arrow
                            if (intensity < 100) intensity++;
                        } else if (key == 66) { // Down arrow
                            if (intensity > 0) intensity--;
                        }
                    }
                }
#endif
                
                // Shared keys for both platforms
                if (key == 'q' || key == 'Q') {
                    running = false;
                } else if (key == '+' || key == '=') {
                    if (intensity < 100) intensity += 5;
                } else if (key == '-' || key == '_') {
                    if (intensity > 0) intensity -= 5;
                } else if (key == '0') {
                    intensity = 0;
                } else if (key == '5') {
                    intensity = 50;
                } else if (key == '9') {
                    intensity = 100;
                }
                
                // Ensure intensity stays within bounds
                if (intensity > 100) intensity = 100;
                if (intensity < 0) intensity = 0;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
public:
    CPUHeater() : running(true), intensity(0) {
        // Detect number of CPU cores
#if defined(_WIN32) && defined(_MSC_VER) // For Windows with Visual C++
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numCores = sysInfo.dwNumberOfProcessors;
#else // For Unix/Linux and MinGW
        numCores = std::thread::hardware_concurrency();
#endif
        
        // Fallback if detection fails
        if (numCores < 1) numCores = 1;
    }
    
    void start() {
        console.clearScreen();
        console.hideCursor();
        
        // Display title and instructions
        console.setTextColor(COLOR_TITLE);
        std::cout << "+-----------------------------------------------+" << std::endl;
        std::cout << "|   Central Processing Unit Crock Pot Utility   |" << std::endl;
        std::cout << "|                    (CPU^2)                    |" << std::endl;
        std::cout << "+-----------------------------------------------+\n" << std::endl;
        console.setTextColor(COLOR_DEFAULT);
        std::cout << "Detected " << numCores << " CPU cores - all will be used for calculations!" << std::endl;
        std::cout << "\nControls:" << std::endl;
        std::cout << "  UP/DOWN arrows or +/- keys: Adjust calculation intensity" << std::endl;
        std::cout << "  0: Minimum intensity (0%), 5: Half intensity (50%), 9: Maximum intensity (100%)" << std::endl;
        std::cout << "  Q: Quit program" << std::endl;
        std::cout << "\n\n\n\n" << std::endl; // Make space for the intensity display
        
        // Start worker threads (one per CPU core)
        for (int i = 0; i < numCores; ++i) {
            threads.emplace_back(&CPUHeater::workerThread, this, i);
        }
        
        // Start input and display threads
        std::thread inputThread(&CPUHeater::handleInput, this);
        std::thread displayThread(&CPUHeater::displayThread, this);
        
        // Wait for input thread to finish (when user quits)
        inputThread.join();
        
        // Cleanup and exit
        running = false;
        displayThread.join();
        
        for (auto& t : threads) {
            t.join();
        }
        
        console.setCursorPosition(0, 10);
        console.setTextColor(COLOR_DEFAULT);
        console.showCursor();
        std::cout << "\nCPU load reduced." << std::endl;
    }
};

int main() {
    CPUHeater heater;
    heater.start();
    return 0;
}