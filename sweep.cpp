#include <iostream>
#include <thread> // For sleep_for
#include <chrono> // For milliseconds
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

    struct Point {
        int r;
        int c;
        double angle;
        double dist; 
    };

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
    void performDownSweep() {
        // ANSI: Hide Cursor
        print("\033[?25l");

        std::string emptyLine(width, ' ');

        for (int i = 1; i <= height; ++i) {
            // Simply overwrite the current line with spaces
            std::cout << "\033[" << i << ";1H";
            std::cout << emptyLine << std::flush;

            // Speed of animation
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Reset Cursor to Top-Left (1,1)
        std::cout << "\033[1;1H";
        
        // ANSI: Show Cursor
        print("\033[?25h");
    }

    // Animation 2: A "Wipe" from left to right (Faster version)
    void performRightSweep() {
        print("\033[?25l"); // Hide cursor
        
        for (int col = 1; col <= width; ++col) {
            for (int row = 1; row <= height; ++row) {
                // Move to current position and erase
                std::cout << "\033[" << row << ";" << col << "H ";
            }
            std::cout << std::flush;
            
            // Speed of animation
            std::this_thread::sleep_for(std::chrono::microseconds(1000)); 
        }

        std::cout << "\033[1;1H"; // Home
        print("\033[?25h"); // Show cursor
    }

    // Animation 3: A "Wipper" circular sweep (back and forth)
    void performWipperSweep() {
        print("\033[?25l"); // Hide cursor

        // Pivot at bottom center
        int pivotR = height;
        int pivotC = width / 2;

        // 1. Collect all screen coordinates and calculate their angle
        std::vector<Point> points;
        points.reserve(width * height);

        for (int r = 1; r <= height; r++) {
            for (int c = 1; c <= width; c++) {
                // Cartesian conversion relative to pivot
                double x = (double)(c - pivotC);
                // Correct aspect ratio: chars are about twice as tall as wide,
                // so we stretch Y to make the sweep look circular rather than flat.
                double y = (double)(height - r) * 2.0; 

                // Angle: atan2(y, x). 
                // Left side (x < 0) -> Angle approaches PI
                // Right side (x > 0) -> Angle approaches 0
                double ang = std::atan2(y, x);
                points.push_back({r, c, ang, 0.0}); // Dist not used here
            }
        }

        // 2. Sort by angle descending (Sweeps from Left/PI to Right/0)
        std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
            return a.angle > b.angle;
        });

        // 3. Process in angular chunks (Pass 1: Left to Right)
        size_t idx = 0;
        size_t total = points.size();
        std::vector<Point> activeWipper;
        
        if (total == 0) return;

        // Angle step size (radians). Larger = faster sweep, blockier look.
        const double angleStep = 0.05; 

        // --- FORWARD SWEEP (Clearing) ---
        while (idx < total) {
            double currentAng = points[idx].angle;
            double nextThreshold = currentAng - angleStep;

            activeWipper.clear();

            // Collect points for the *new* chunk
            while (idx < total && points[idx].angle > nextThreshold) {
                activeWipper.push_back(points[idx]);
                idx++;
            }

            // Just erase the chunk immediately (no brush drawn)
            for (const auto& p : activeWipper) {
                std::cout << "\033[" << p.r << ";" << p.c << "H "; 
            }
            std::cout << std::flush;

            // Delay
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        // --- BACKWARD SWEEP (Returning) ---
        // We go backwards through the points vector to sweep Right to Left.
        long r_idx = (long)total - 1;

        while (r_idx >= 0) {
            double currentAng = points[r_idx].angle;
            double nextThreshold = currentAng + angleStep;

            activeWipper.clear();

            // Collect points moving backwards
            while (r_idx >= 0 && points[r_idx].angle < nextThreshold) {
                activeWipper.push_back(points[r_idx]);
                r_idx--;
            }

            // Erase chunk
            for (const auto& p : activeWipper) {
                std::cout << "\033[" << p.r << ";" << p.c << "H "; 
            }
            std::cout << std::flush;

            // Slightly faster return
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        std::cout << "\033[1;1H"; 
        print("\033[?25h");
    }

    // Animation 4: Black Hole (Sucks everything into center)
    void performBlackHoleSweep() {
        print("\033[?25l"); // Hide cursor

        // Center of the screen
        int pivotR = height / 2;
        int pivotC = width / 2;

        std::vector<Point> points;
        points.reserve(width * height);

        for (int r = 1; r <= height; r++) {
            for (int c = 1; c <= width; c++) {
                double dy = (double)(r - pivotR) * 2.0; // Aspect correction
                double dx = (double)(c - pivotC);
                double dist = std::sqrt(dx*dx + dy*dy);
                // Angle used for swirl effect
                double ang = std::atan2(dy, dx); 
                points.push_back({r, c, ang, dist});
            }
        }

        // Sort for Spiral Effect (Outside In)
        std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
            // Factor 1.5 helps interleave the angle with distance to spiral
            double scoreA = a.dist + (a.angle / 3.14159);
            double scoreB = b.dist + (b.angle / 3.14159);
            return scoreA > scoreB;
        });

        int counter = 0;
        for (const auto& p : points) {
            // Move to position and erase (Overwrite with space)
            std::cout << "\033[" << p.r << ";" << p.c << "H "; 
            
            // Optimization: Flush every 10 characters to keep animation smooth
            if (++counter % 10 == 0) {
                 std::cout << std::flush;
                 std::this_thread::sleep_for(std::chrono::microseconds(400));
            }
        }
        std::cout << std::flush;

        std::cout << "\033[1;1H"; 
        print("\033[?25h");
    }
};

int main(int argc, char* argv[]) {
    TerminalSweeper sweeper;
    std::string mode = "down"; // Default behavior

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "-r" || arg == "--right") {
            mode = "right";
        } else if (arg == "-w" || arg == "--wipper") { 
            mode = "wipper";
        } else if (arg == "-b" || arg == "--blackhole") { 
            mode = "blackhole";
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  -d, --down      Vertical scanner erase (default)\n"
                      << "  -r, --right     Horizontal wipe erase\n"
                      << "  -w, --wipper    Circular wipper erase (back and forth)\n"
                      << "  -b, --blackhole Spiral vacuum erase\n"
                      << "  -h, --help      Show help message\n";
            return 0;
        } else if (arg != "-d" && arg != "--down") {
            std::cerr << "Unknown option: " << arg << "\n"
                      << "Try '" << argv[0] << " --help' for more information.\n";
            return 1;
        }
    }
    
    // Execute chosen mode
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
