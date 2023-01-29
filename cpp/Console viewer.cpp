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
        static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count())
    };
    std::mt19937 prng{sq};
    constexpr auto cinlim{ std::numeric_limits<std::streamsize>::max() };
    constexpr long double uintend{ (long double)std::numeric_limits<unsigned>::max() + 1 };

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
    CellGrid pgrid(width, height, 3076);
    CellGrid grid(pgrid, 13, 30483, 1234, CellGrid::torus);
    grid.rule = 3076;
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
    } while (!grid.rule.readstr(rule));

    char inStr[16];
    do {
        cout << "Random fill percentage (%): ";
        if (cin >> inStr) {
            if (*inStr == '.' || inStr[1] == '.' || inStr[2] == '.') {
                long double fillD{};
                std::from_chars(inStr, inStr + 16, fillD); // is there a way to make this round instead of floor?
                fillD /= 100;
                if (fillD && fillD >=0 && fillD <= 1) { // >=0 makes an angry face
                    retry = false;
                    unsigned threshold{ static_cast<unsigned>(fillD * uintend) },
                            threshold2{ static_cast<unsigned>((fillD * uintend - (long double)threshold) * uintend) };
                    if (threshold) {
                        unsigned res;
                        for (int x, y{ height / 8 }, xlim{ 7 * width / 8 }; y != 7 * height / 8; ++y)
                            for (x = width / 8; x != xlim; ++x) res = prng(),
                                grid(x, y, res < threshold || res == threshold && prng() < threshold2);
                    } else if (fillD < 1) retry = true;
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
    } while (simlen < 0 || simlen > 65535i32);

    cout << "\nStart grid: \n" << grid;
    cout << "\n\nFinal grid - Generation " << simlen << ":\n"
         << (grid += simlen);
    
    cout << "\n\nProgram completed\n";
    char temp;
    cin >> temp;
    return 0;
}
