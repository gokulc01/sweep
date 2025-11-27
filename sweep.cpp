#include <iostream>
#include <thread> // For sleep_for
#include <chrono> // For milliseconds
#include <string>

// Platform specific includes for terminal size
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

// Class to handle terminal operations
class TerminalSweeper {
private:
    int width;
    int height;

public:
    TerminalSweeper() {
        updateDimensions();
    }

    // specific logic to get terminal width/height
    void updateDimensions() {
        width = 80;  // Fallback defaults
        height = 24;

        #ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
                width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            }
        #else
            struct winsize w;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
                width = w.ws_col;
                height = w.ws_row;
            }
        #endif
    }

    // Helper to print ANSI escape code
    void print(const std::string& str) {
        std::cout << str;
    }

    // Animation 1: A "Scanner" bar moving down
    void performScanSweep() {
        // ANSI: Hide Cursor
        print("\033[?25l");

        std::string emptyLine(width, ' ');
        std::string scannerLine(width, '*');

        // Color the scanner line Green (ANSI 32) and Bold (1)
        // \033[1;32m   = Start Green
        // \033[0m      = Reset
        std::string coloredScanner = "\033[1;32m" + scannerLine + "\033[0m";

        for (int i = 1; i <= height; ++i) {
            // 1. Erase the PREVIOUS line (if not the first)
            if (i > 1) {
                // Move cursor to previous row (i-1), column 1
                std::cout << "\033[" << (i - 1) << ";1H"; 
                std::cout << emptyLine;
            }

            // 2. Draw the SCANNER on the CURRENT line
            // Move cursor to current row (i), column 1
            std::cout << "\033[" << i << ";1H";
            std::cout << coloredScanner << std::flush;

            // Speed of animation (adjust milliseconds to make faster/slower)
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }

        // Clean up the very last scanner line
        std::cout << "\033[" << height << ";1H";
        std::cout << emptyLine;

        // Reset Cursor to Top-Left (1,1)
        std::cout << "\033[1;1H";
        
        // ANSI: Show Cursor
        print("\033[?25h");
    }

    // Animation 2: A "Wipe" from left to right (Faster version)
    void performWipeRight() {
        print("\033[?25l"); // Hide cursor
        
        // A vertical bar used for wiping
        std::string wiper = "\033[1;36m|\033[0m"; // Cyan pipe

        for (int col = 1; col <= width; ++col) {
            for (int row = 1; row <= height; ++row) {
                // Move to current position
                std::cout << "\033[" << row << ";" << col << "H";
                std::cout << wiper;
                
                // Erase position behind it (col - 1)
                if (col > 1) {
                    std::cout << "\033[" << row << ";" << (col - 1) << "H";
                    std::cout << " "; 
                }
            }
            std::cout << std::flush;
            // Faster delay needed because we loop columns * rows
            // Note: This can be slow on large terminals due to flush overhead
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        // Clean up last column
        for (int row = 1; row <= height; ++row) {
            std::cout << "\033[" << row << ";" << width << "H";
            std::cout << " ";
        }

        std::cout << "\033[1;1H"; // Home
        print("\033[?25h"); // Show cursor
    }
};

int main(int argc, char* argv[]) {
    // Actual Logic
    TerminalSweeper sweeper;
    
    // Choose your animation style here:
    // sweeper.performScanSweep(); 
    sweeper.performWipeRight(); // Uncomment to try the other style

    return 0;
}
