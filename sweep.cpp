#include <iostream>
#include <thread> // For sleep_for
#include <chrono> // For milliseconds, microseconds
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

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
    
    // Target duration for animations
    int targetDurationMs; 

    struct Point {
        int r;
        int c;
        double angle;
        double dist; 
    };

public:
    TerminalSweeper(int duration = 500) : targetDurationMs(duration) {
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

    // Animation 1: Vertical Scan
    void performDownSweep() {
        print("\033[?25l"); // Hide Cursor

        std::string emptyLine(width, ' ');

        // Calculate delay to ensure total time is targetDurationMs
        int stepDelay = std::max(1, targetDurationMs / height);

        for (int i = 1; i <= height; ++i) {
            std::cout << "\033[" << i << ";1H";
            std::cout << emptyLine << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(stepDelay));
        }

        std::cout << "\033[1;1H"; 
        print("\033[?25h"); // Show Cursor
    }

    // Animation 2: Horizontal Wipe
    void performRightSweep() {
        print("\033[?25l"); 
        
        // Calculate delay in microseconds for better precision on wide terminals
        int stepDelayUs = (targetDurationMs * 1000) / width;

        for (int col = 1; col <= width; ++col) {
            for (int row = 1; row <= height; ++row) {
                std::cout << "\033[" << row << ";" << col << "H ";
            }
            std::cout << std::flush;
            std::this_thread::sleep_for(std::chrono::microseconds(stepDelayUs)); 
        }

        std::cout << "\033[1;1H"; 
        print("\033[?25h"); 
    }

    // Animation 3: Wipper (Circular back and forth)
    void performWipperSweep() {
        print("\033[?25l"); 

        int pivotR = height;
        int pivotC = width / 2;
        std::vector<Point> points;
        points.reserve(width * height);

        for (int r = 1; r <= height; r++) {
            for (int c = 1; c <= width; c++) {
                double x = (double)(c - pivotC);
                double y = (double)(height - r) * 2.0; 
                double ang = std::atan2(y, x);
                points.push_back({r, c, ang, 0.0});
            }
        }

        std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
            return a.angle > b.angle;
        });

        // Calculate Timing
        const double angleStep = 0.05; 
        // Approx steps in one pass (PI / 0.05)
        int stepsPerPass = (int)(3.14159 / angleStep);
        if (stepsPerPass == 0) stepsPerPass = 1;

        // Total time divided by 2 passes (Forward + Back)
        int passDuration = targetDurationMs / 2;
        int stepDelay = std::max(1, passDuration / stepsPerPass);

        // --- Pass 1: Forward ---
        size_t idx = 0;
        size_t total = points.size();
        std::vector<Point> activeWipper;
        
        while (idx < total) {
            double currentAng = points[idx].angle;
            double nextThreshold = currentAng - angleStep;

            activeWipper.clear();
            while (idx < total && points[idx].angle > nextThreshold) {
                activeWipper.push_back(points[idx]);
                idx++;
            }

            for (const auto& p : activeWipper) {
                std::cout << "\033[" << p.r << ";" << p.c << "H "; 
            }
            std::cout << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(stepDelay));
        }

        // --- Pass 2: Backward ---
        long r_idx = (long)total - 1;
        while (r_idx >= 0) {
            double currentAng = points[r_idx].angle;
            double nextThreshold = currentAng + angleStep;

            activeWipper.clear();
            while (r_idx >= 0 && points[r_idx].angle < nextThreshold) {
                activeWipper.push_back(points[r_idx]);
                r_idx--;
            }

            for (const auto& p : activeWipper) {
                std::cout << "\033[" << p.r << ";" << p.c << "H "; 
            }
            std::cout << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(stepDelay));
        }

        std::cout << "\033[1;1H"; 
        print("\033[?25h");
    }

    // Animation 4: Black Hole (Spiral Vacuum)
    void performBlackHoleSweep() {
        print("\033[?25l"); 

        int pivotR = height / 2;
        int pivotC = width / 2;

        std::vector<Point> points;
        points.reserve(width * height);

        for (int r = 1; r <= height; r++) {
            for (int c = 1; c <= width; c++) {
                double dy = (double)(r - pivotR) * 2.0; 
                double dx = (double)(c - pivotC);
                double dist = std::sqrt(dx*dx + dy*dy);
                double ang = std::atan2(dy, dx); 
                points.push_back({r, c, ang, dist});
            }
        }

        std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
            double scoreA = a.dist + (a.angle / 3.14159);
            double scoreB = b.dist + (b.angle / 3.14159);
            return scoreA > scoreB;
        });

        // Calculate Timing
        size_t totalPoints = points.size();
        int batchSize = 10;
        size_t totalBatches = totalPoints / batchSize;
        if (totalBatches == 0) totalBatches = 1;

        // Delay per batch in microseconds to fit total duration
        int batchDelayUs = (targetDurationMs * 1000) / totalBatches;

        int counter = 0;
        for (const auto& p : points) {
            std::cout << "\033[" << p.r << ";" << p.c << "H "; 
            
            if (++counter % batchSize == 0) {
                 std::cout << std::flush;
                 std::this_thread::sleep_for(std::chrono::microseconds(batchDelayUs));
            }
        }
        std::cout << std::flush;

        std::cout << "\033[1;1H"; 
        print("\033[?25h");
    }
};

int main(int argc, char* argv[]) {
    int duration = 500; // Default 500ms
    std::string mode = "down"; 

    // Parse arguments loop
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-t") {
            if (i + 1 < argc) {
                try {
                    duration = std::stoi(argv[++i]);
                } catch (...) {
                    std::cerr << "Error: Invalid duration provided.\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: -t requires a duration (ms).\n";
                return 1;
            }
        } else if (arg == "-r" || arg == "--right") {
            mode = "right";
        } else if (arg == "-w" || arg == "--wipper") { 
            mode = "wipper";
        } else if (arg == "-b" || arg == "--blackhole") { 
            mode = "blackhole";
        } else if (arg == "-d" || arg == "--down") {
            mode = "down";
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  -d, --down      Vertical scanner erase (default)\n"
                      << "  -r, --right     Horizontal wipe erase\n"
                      << "  -w, --wipper    Circular wipper erase (back and forth)\n"
                      << "  -b, --blackhole Spiral vacuum erase\n"
                      << "  -t <ms>         Animation duration in milliseconds (default: 500)\n"
                      << "  -h, --help      Show help message\n";
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n"
                      << "Try '" << argv[0] << " --help' for more information.\n";
            return 1;
        }
    }
    
    // Initialize sweeper with parsed duration
    TerminalSweeper sweeper(duration);

    if (mode == "right") {
        sweeper.performRightSweep();
    } else if (mode == "wipper") {
        sweeper.performWipperSweep();
    } else if (mode == "blackhole") {
        sweeper.performBlackHoleSweep();
    } else {
        sweeper.performDownSweep(); 
    }

    return 0;
}
