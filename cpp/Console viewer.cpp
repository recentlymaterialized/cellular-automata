#include <iostream>
#include <string>
#include <cstdint>
#include <charconv>
#include <random>
#include <chrono>
#include "CellGrid.h"

using std::cin;
using std::cout;

int main() {
    std::random_device rd;
    std::seed_seq sq{
        rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(),
        static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count())
    };
    std::mt19937 prng{sq};
    constexpr auto cinlim{ std::numeric_limits<std::streamsize>::max() };

    char rule[21];
    std::int_fast16_t width{}, height{};
    std::int_fast32_t simlen{-1};
    cout << "Cellular automata simulation\n";
    do {
        cout << "Width: ";
        if (!(cin >> width)) cin.clear();
        cin.ignore(cinlim, '\n');
    } while (width < 1 || width > 1855);
    do {
        cout << "Height: ";
        if (!(cin >> height)) cin.clear();
        cin.ignore(cinlim, '\n');
    } while (height < 1 || height > 1855);

    /** // testing various parts of CellGrid; this code reveals a bunch of issues that I have no idea how to fix
    CellGrid pgrid(width, height, 3076l);
    CellGrid grid(pgrid, 13, 30483, 1234, CellGrid::torus);
    grid.rule = 3076l;
    grid.resize(width, height);
    /*/
    CellGrid grid(width, height, "", CellGrid::torus);
    //*/
    bool retry{};
    do {
        cout << (retry ? "Invalid rule\nRule: " : "Rule: ");
        if (!(cin >> rule)) cin.clear();
        cin.ignore(cinlim, '\n');
        retry = true;
    } while (!(grid.rule = rule));

    char inStr[16];
    do {
        cout << "Random fill percentage (%): ";
        if (cin >> inStr) {
            if (*inStr == '.' || inStr[1] == '.' || inStr[2] == '.') {
                long double fillD{};
                std::from_chars(inStr, inStr + 16, fillD);
                fillD /= 100.l;
                if (fillD && fillD >=0 && fillD <= 1.l) { // >=0 makes an angry face
                    retry = false;
                    unsigned int threshold{ static_cast<unsigned int>(fillD * ((long double)UINT_MAX + 1.l)) };
                    // is UINT_MAX in the official C++ specification?
                    if (threshold) for (int x, y{ height / 8 }, xlim{ 7 * width / 8 }; y != 7 * height / 8; ++y)
                        for (x = width / 8; x != xlim; ++x) grid(x, y, prng() < threshold);
                    else if (fillD < 1) retry = true;
                    else for (int x, y{ height / 8 }; y != 7 * height / 8; ++y)
                        for (x = width / 8; x != 7 * width / 8; ++x) grid(x, y, true);
                }
            }
            else {
                int fillI{};
                std::from_chars(inStr, inStr + 8, fillI);
                if (fillI && fillI >=0 && fillI < 101) {
                    if (fillI == 100) for (int x, y{ height / 8 }; y != 7 * height / 8; ++y)
                        for (x = width / 8; x != 7 * width / 8; ++x) grid(x, y, true);
                    else if (fillI == 50) {
                        std::uniform_int_distribution bin{0, 1};
                        for (int x, y{ height / 8 }; y != 7 * height / 8; ++y)
                            for (x = width / 8; x != 7 * width / 8; ++x) grid(x, y, bin(prng));
                    }
                    else {
                        std::uniform_int_distribution cent{0, 99};
                        for (int x, y{ height / 8 }; y != 7 * height / 8; ++y)
                            for (x = width / 8; x != 7 * width / 8; ++x) grid(x, y, cent(prng) < fillI);
                    }
                    retry = false;
                }
            }
        }
        else cin.clear();
        cin.ignore(cinlim, '\n');
    } while (retry);

    do {
        cout << "Simulation length: ";
        if (!(cin >> simlen)) cin.clear();
        cin.ignore(cinlim, '\n');
    } while (simlen < 0 || simlen > 65535l);

    cout << "\nStart grid: \n" << grid;
    cout << "\n\nFinal grid - Generation " << simlen << ":\n"
         << (grid += simlen);
    
    cout << "\n\nProgram completed\n";
    char temp;
    cin >> temp;
    return 0;
}

// for (unsigned char i{}; i++ != 255;) cout << (int)i << ' ' << i << '\n';
