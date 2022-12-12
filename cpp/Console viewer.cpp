#include <iostream>
#include <string>
#include <cstdint>
#include <random>
#include <chrono>
#include "CellGrid.h"

using std::cin;
using std::cout;

int main() {
    std::random_device rd;
    std::seed_seq sq{
        rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(),
        static_cast<std::uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count())
    };
    std::mt19937 prng{sq};
    std::uniform_int_distribution bin{0, 1};
    constexpr auto cinlim{ std::numeric_limits<std::streamsize>::max() };

    char rule[21];
    std::int_fast16_t width{1856}, height{1856};
    std::int_fast32_t simlen{65536l};
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

    CellGrid grid(width, height);
    bool retry{};
    do {
        cout << (retry ? "Invalid rule\nRule: " : "Rule: ");
        if (!(cin >> rule)) cin.clear();
        cin.ignore(cinlim, '\n');
        retry = true;
    } while (!(grid.rule = rule));
    do {
        cout << "Simulation length: ";
        if (!(cin >> simlen)) cin.clear();
        cin.ignore(cinlim, '\n');
    } while (simlen < 0 || simlen > 65535l);

    for (int x, y{ height / 8 }; y != 7 * height / 8; ++y)
        for (x = width / 8; x != 7 * width / 8; ++x) grid(x, y, bin(prng));

    grid += simlen;
    cout << '\n' << grid << "\n\nProgram completed\n";

    char temp;
    cin >> temp;
    return 0;
}

// for (unsigned char i{}; i++ != 255;) cout << (int)i << ' ' << i << '\n';
