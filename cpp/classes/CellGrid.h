#ifndef recentlymaterialized_CellGrid
#define recentlymaterialized_CellGrid
#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <string>

// DISCLAIMER
// i am a novice at c++. do not actually use this.


/***Usage***
> Instances of this class hold a grid of cells and can
  simulate basic cellular automata in a range of rules
> C++20-compliant compiler required
> Freely extensible: no private members

// Constructors:
CellGrid(width, height, rule? = Life, edgeBehavior? = CellGrid::plane, generation? = 0)
CellGrid(initialGrid, width, height, rule?, edgeBehavior?, generation?)
CellGrid(initialGrid, rule?, edgeBehavior?, generation?)
// some constructors are not available due to conflicts.
   most of the constructors currently have issues that i will try to fix

CellGrid myGrid(1024, 512, "B3/S23"); // New empty 1024x512 Life grid
bool array2d[1024][96];
CellGrid slimGrid(array2d, 96, 1024); // New 96x1024 Life grid from array2d
CellGrid copyGrid(myGrid, '', 0, 12); // Makes a copy of myGrid with generation=12
                                    (a two-dimensional container class with a
                                    .size() member function may alternatively
                                     be used as initialGrid here) -- currently not working because i
                                                                     have no idea how templates work
// CellGrid member functions:

myGrid(30, 20) // Gets value of cell at x:30, y:20
myGrid(30, 20, 1) // Sets cell at x:30, y:20 to ON
myGrid( 8, 80, 0) // Sets cell at x:8, y:80 to OFF

myGrid++; // Advances grid one generation (may be prefix or postfix)
myGrid += 16; // Advances 16 generations

myGrid.gen = 4001;           // Generation, edge behavior, and
myGrid.edge = CellGrid::torus;  rule may be changed freely as
myGrid.rule = "B36/S23";        desired (note: B0 not allowed)
myGrid.rule = copyGrid.rule = 0b00000110000100100i32; // bits 0-7 are birth conditions for 1-8 neighbors;
                                                     bits 8-16 are survival conditions for 0-8 neighbors
myGrid.width() || myGrid.height() // The dimensions of the grid are accessible as signed 32-bit integers
myGrid.resize(512, 1024) // The grid is resizable, with options to center the ON cells or clear them
myGrid.resize(512, 1024, false) // Clears the grid before resizing
myGrid.resize(512, 1024, true, true) // Centers the grid after resizing
// Warning: The resize function currently has multiple issues and should not be used

myGrid.count()      // Returns the number of ON  cells in the grid
myGrid.count(false) // Returns the number of OFF cells in the grid
myGrid.data() // Returns const bool* const* access to the internal grid (array of rows of bools) [deleted on resize]

myGrid.print()     // Prints grid to the console via std::cout
myGrid.print(true) // Prints grid and generation number
  (The grid is printed using char values 220, 223, 219, and space to display two cells per character)
myGrid.printSafe(false, '#') // Prints grid without special characters (4x larger: two characters per cell)

myGrid = std::move(oldGrid); // move semantics are allowed, but the rvalue CellGrid
                                (oldGrid) is left in an unusable state
oldGrid.isValid() // returns false if contents have been moved away, otherwise true

// CellGrid::Rule member functions:
myGrid.rule.b(n) // Returns true if an OFF cell becomes ON with n neighbors (birth)
myGrid.rule.s(n) // Returns true if an ON cell stays ON with n neighbors (survival)
myGrid.rule.d(n) // Returns true if an ON cell becomes OFF with n neighbors (death)
myGrid.rule.birthArr(),
myGrid.rule.deathArr() // Returns array of 9 const bools for each possible number of neighbors (0-8)
(unsigned long)(myGrid.rule) // Converts to a 17-bit integer of B/S behavior (Life = 3076)
myGrid.rule.cstr() or (const char*)(myGrid.rule) // Returns a human-readable C-style string ("B3/S23")
                                           (allocated & created on first run; dies when rule changes or dies)
myGrid.rule.str() or (std::string)(myGrid.rule) // Creates a human-readable std::string ("B3/S23")
myGrid.rule.strv() or (std::string_view)(myGrid.rule) // Creates a human-readable std::string_view ("B3/S23")

myGrid.readstr("...") // Sets rule to rulestring "...", returns true if successful, false if rulestring is invalid
myGrid.rule = "..." // Sets rule to rulestring "...", throws std::invalid_argument if rulestring is invalid

// You can change the topology, or "edge behavior", of the grid: whether cells at one
    edge of the grid wrap around to meet cells on another edge.
// There are currently 6 options for edge behavior (of type CellGrid::Topology):
   Identifier      Value     Description
CellGrid::plane   |  0  |  No edge joining. Cells outside the grid are considered OFF.
CellGrid::torus   |  1  |  Cells on the left/right edges are adjacent to the cell on the opposite edge on the same
-                 |     |    row. Cells on the top/bottom edges are adjacent to the opposite cell on their column.
CellGrid::bottle, |  2  |  Similar to torus, but cells on the top/bottom edges are adjacent to the opposite cell
CellGrid::vbottle |     |    on the opposite column. CellGrid::bottle is an alias for CellGrid::vbottle.
CellGrid::hbottle |  3  |  A horizontal version of the Klein bottle. (vbottle is usually faster)
CellGrid::cross   |  4  |  Cross-surface: cells on the L/R and T/B edges meet on the opposite row/column.
CellGrid::sphere  |  5  |  Similar to torus, but adjacent edges meet, rather than opposite edges. (width = height)
The edge topology may be included in the grid's constructor, or modified by setting grid.edge or grid.topology.
Additionally, joined edges may be "shifted" relative to each other (see diagram at bottom of file) using the
.shift() member function. This has no effect on planes.

// Examples:
CellGrid torusGrid(50, 50, "", CellGrid::torus) // rule defaults to Life
CellGrid sphereGrid(torusGrid, "", CellGrid::sphere) // a copy of torusGrid but with sphere topology
sphereGrid.edge = CellGrid::cross // sphereGrid is now a cross-surface
sphereGrid.edge = CellGrid::Topology{5} // sphereGrid is now a sphere, created from the code 5 (explicit)
torusGrid.shift(2) // the top & bottom edges of torusGrid are now shifted 2 cells relative to each other
                   // (imagine a rectangular tiling of CellGrids, where each next row is shifted 2 cells to the right)
torusGrid.shift(-4) // torusGrid is now shifted horizontally 4 cells in the opposite direction
torusGrid.shift(2, true) // torusGrid is now shifted vertically 2 cells
	[ edge behavior is currently unfinished; the only functional
	  options are plane, torus, and horizontally shifted torus ]
// The names of the edge behaviors (as C-style strings) are stored in the non-const static member
   CellGrid::Topology::names. The name of a Topology instance may be accessed using .cstr() [c-style
   string], .str() [std::string], or .strv [std::string_view].

// CellGrid::iterator and ::const_iterator classes are available, but the non-const ::iterator
     may only be used before the grid has started running the CA (before operator++ is called).
   If you would like to use the non-const ::iterator after running a few generations, you can
     .pause() the grid, use the iterator, and then resume it by calling operator++ again.
   CellGrid::iterator::nextRow, ::prevRow, and ::moveRow(int32) may be used to directly
       navigate vertically. ++ (increment) is faster than -- (decrement).

// Basic overloads for std::ostream::operator<< are provided for CellGrid, CellGrid::Rule, and CellGrid::Topology.

***********/

/*
  to do:
  - fix the issues with the constructors
  - integrate some std::algorithms to make simulations faster (or learn parallelism)
  - add options for different edge behaviors (in progress!)
  - support some new families of rules (isotropic, generations, LtL)
  - use this to make something with graphics (help)
  - look up stuff about optimizing cellular automata

  HOW IS GOLLY SO FAST?????
  i read through the quicklife code and understood nothing :'(
*/

// source:  https://github.com/recentlymaterialized/cellular-automata/blob/main/cpp/classes/CellGrid.h
// license: https://github.com/recentlymaterialized/cellular-automata/blob/main/LICENSE  (MIT license)

class CellGrid {
	using u32 = std::uint_fast32_t; // B/S rule codes (currently use 17 bits)
	using i32 = std::int_fast32_t; // grid dimensions are 32-bit integers
	using i64 = std::int_fast64_t; // why not
	using uch = unsigned char;
	using stv = std::string_view;
public:
	static_assert(sizeof(i32) <= sizeof(size_t));
	static u32 defaultRule; // "B3/S23" == 0b00000110000000100i32 == 3076 == Conway's Game of Life
protected:
	bool** dat, ** act, // stores grid and active cells
	     * dst{nullptr}, * dpv{nullptr}, * dcr{nullptr}, // helper arrays for calculations
	     * acr{nullptr}, * anx{nullptr}, * als{nullptr},
	     * dlf{nullptr}, * drg{nullptr};
	bool started{}; // if false, act is treated as uninitialized
	mutable uch* pRow{nullptr}; // helper array for printing
	i32 w, h;
	i32 esh{};
	bool vsh{};
	// Simulation algorithm: the only important part of the class
	// it's very slow... i need to find ways to optimize it
	virtual void algorithm(const bool* birth, const bool* death, i32 (CellGrid::*edgeCount)(i32, i32),
	                       void (CellGrid::*edgeSet)(i32, i32, bool), bool flicker) {
		int neighbors;
		i32 wm1{ w - 1 }, hm1{ h - 1 };
		for (i32 x{}; x != w; ++x) if (acr[x]) {
			neighbors = (this->*edgeCount)(x, 0); // i hate this syntax. https://stackoverflow.com/a/2402607 tysm
			if (dcr[x]) { if (death[neighbors]) (this->*edgeSet)(x, 0, false); }
			else if (birth[neighbors]) (this->*edgeSet)(x, 0, true);
		}
		if (h != 1) { [[likely]];
		bool* dnx, * asp, * asc, * asn;
		for (i32 x, y{1}, xm1, xp1; y != hm1; ++y) {
			dnx = dat[y + 1];
			asp = act[y - 1], asc = act[y], asn = act[y + 1];
			for (i32 i{}; i != w; ++i) {
				dpv[i] = dcr[i];
				dcr[i] = dat[y][i];
				acr[i] = anx[i];
				anx[i] = act[y+1][i]; // y < h-1
				if (y+1 != hm1) act[y+1][i] = false;
			}
			if (*acr) {
				neighbors = (this->*edgeCount)(0, y);
				if (*dcr) { if (death[neighbors]) (this->*edgeSet)(0, y, false); }
				else if (birth[neighbors]) (this->*edgeSet)(0, y, true);
			}
			if (acr[wm1]) {
				neighbors = (this->*edgeCount)(wm1, y);
				if (dcr[wm1]) { if (death[neighbors]) (this->*edgeSet)(wm1, y, false); }
				else if (birth[neighbors]) (this->*edgeSet)(wm1, y, true);
			}
			if (w != 1) {
				xm1 = 0; x = 1; xp1 = 2;
				if (flicker) {for (; x != wm1; ++x, ++xm1, ++xp1) if (acr[x]) {
					neighbors = dpv[xm1] + dpv[x] + dpv[xp1]
					          + dcr[xm1]          + dcr[xp1]
					          + dnx[xm1] + dnx[x] + dnx[xp1];
					if (dcr[x]) {
						if (death[neighbors]) dat[y][x] = false;
						else continue;
					}
					else {
						if (birth[neighbors]) dat[y][x] = true;
						else continue;
					}
					asp[xm1] = asp[x] = asp[xp1] =
					asc[xm1] = asc[x] = asc[xp1] =
					asn[xm1] = asn[x] = asn[xp1] = true;
				}} else for (; x != wm1; ++x, ++xm1, ++xp1) if (acr[x]) {
					neighbors = dpv[xm1] + dpv[x] + dpv[xp1]
					          + dcr[xm1]          + dcr[xp1]
					          + dnx[xm1] + dnx[x] + dnx[xp1];
					if (dcr[x]) {
						if (death[neighbors]) dat[y][x] = false;
						else continue;
					}
					else {
						if (birth[neighbors]) dat[y][x] = true;
						else continue;
					}
					asp[xm1] = asp[x] = asp[xp1] =
					asc[xm1]          = asc[xp1] =
					asn[xm1] = asn[x] = asn[xp1] = true;
					// if Rule.canFlicker() is false, the current cell does
					//   not have to be activated for the next generation
				}
			}
		}
		for (i32 i{}; i != w; ++i) {
			dpv[i] = dcr[i];
			dcr[i] = dat[hm1][i];
			acr[i] = als[i];
		}
		for (i32 x{}; x != w; ++x) if (acr[x]) {
			neighbors = (this->*edgeCount)(x, hm1);
			if (dcr[x]) { if (death[neighbors]) (this->*edgeSet)(x, hm1, false); }
			else if (birth[neighbors]) (this->*edgeSet)(x, hm1, true);
		}
		}
	}
	void algprep(i32 (CellGrid::*&edgeCount)(i32, i32), void (CellGrid::*&edgeSet)(i32, i32, bool), bool flicker) {
		// member function pointers, passed by reference. this syntax is very confusing :(
		if (!dst) {
			dst = new bool[w];
			dpv = new bool[w];
			dcr = new bool[w];
			acr = new bool[w];
			anx = new bool[w];
			als = new bool[w];
		}
		switch (edge.id) {
		case 0:
			edgeCount = &CellGrid::planeCount;
			break;
		case 1:
			edgeCount = esh ? &CellGrid::shTorusCount : &CellGrid::torusCount;
			break;
		case 5:
			edgeCount = esh ? &CellGrid::shSphereCount : &CellGrid::sphereCount;
			break;
		default:
			edgeCount = esh ? &CellGrid::shCrossesCount : &CellGrid::crossesCount;
			break;
		}
		if (flicker) {
			switch (edge.id) {
			case 0:
				edgeSet = &CellGrid::planeSetFl;
				break;
			case 1:
				edgeSet = esh ? &CellGrid::shTorusSetFl : &CellGrid::torusSetFl;
				break;
			case 5:
				edgeSet = esh ? &CellGrid::shSphereSetFl : &CellGrid::sphereSetFl;
				break;
			default:
				edgeSet = esh ? &CellGrid::shCrossesSetFl : &CellGrid::crossesSetFl;
				break;
			}
		}
		else {
			switch (edge.id) {
			case 0:
				edgeSet = &CellGrid::planeSet;
				break;
			case 1:
				edgeSet = esh ? &CellGrid::shTorusSet : &CellGrid::torusSet;
				break;
			case 5:
				edgeSet = esh ? &CellGrid::shSphereSet : &CellGrid::sphereSet;
				break;
			default:
				edgeSet = esh ? &CellGrid::shCrossesSet : &CellGrid::crossesSet;
				break;
			}
		}
		i32 hm1{ h - 1 };
		if (!started) {
			i32 wm1{ w - 1 };
			for (i32 i{}, j; i != h; ++i) {
				bool* asp{ i ? act[i - 1] : act[hm1] },
					* asc{ act[i] },
					* asn{ i == hm1 ? *act : act[i + 1] };
				for (j = 0;;) {
					if (i && i != hm1 && j && j != -1) {
						//if (flicker) {
						if (j != wm1 && j != w && dat[i][j])
							asp[j-1] = asp[j] = asp[j+1] =
							asc[j-1] = asc[j] = asc[j+1] =
							asn[j-1] = asn[j] = asn[j+1] = true;
						else if (--j != wm1 && dat[i][j])
							asp[j-1] = asp[j] = asp[j+1] =
							asc[j-1] = asc[j] = asc[j+1] =
							asn[j-1] = asn[j] = asn[j+1] = true;
						else if (dat[i][--j])
							asp[j-1] = asp[j] = asp[j+1] =
							asc[j-1] = asc[j] = asc[j+1] =
							asn[j-1] = asn[j] = asn[j+1] = true;
						j += 3;
						if (j == w + 1) j = -1;
						/*}
						else {
						if (dat[i][j])
						asp[j-1] = asp[j] = asp[j+1] =
						asc[j-1]          = asc[j+1] =
						asn[j-1] = asn[j] = asn[j+1] = true;
						++j;
						if (j == wm1) j = -1;
						}*/
					}
					else {
						if (j == -1) j = wm1;
						i32 jm1{ j ? j - 1 : wm1 },
							jp1{ j == wm1 ? 0 : j + 1 };
						if (flicker) {
							if (dat[i][j]) {
								asp[jm1] = asp[j] = asp[jp1] =
									asc[jm1] = asc[j] = asc[jp1] =
									asn[jm1] = asn[j] = asn[jp1] = true;
								if (i && i != hm1) j += 2;
							}
						}
						else if (dat[i][j])
							asp[jm1] = asp[j] = asp[jp1] =
							asc[jm1]          = asc[jp1] =
							asn[jm1] = asn[j] = asn[jp1] = true;
						if (j == wm1) break;
						++j;
					}
				}
			}
			started = true;
		}
		for (i32 i{}; i != w; ++i) {
			dst[i] =
				dcr[i] = dat[0][i];
			dpv[i] = dat[hm1][i];
			acr[i] = act[0][i];
			als[i] = act[hm1][i];
			act[0][i] = act[hm1][i] = false;
			if (h != 1) {
				anx[i] = act[1][i];
				act[1][i] = false;
			}
		}
	}
	i32 planeCount(i32 x, i32 y) {
		if (y) {if (y == h - 1) { // would one return statement and a bunch of ternary ?: operators be faster?
			if (x) {
				if (x == w - 1) [[unlikely]] return dpv[x - 1] + dpv[x] + dcr[x - 1];
				else [[likely]] return dpv[x - 1] + dpv[x] + dpv[x + 1] + dcr[x - 1] + dcr[x + 1];
			} else [[unlikely]] return dpv[x] + dpv[x+1] + dcr[x+1];
		} else {
			if (x) return dpv[x-1] + dpv[x] + dcr[x-1] + dat[y+1][x-1] + dat[y+1][x];
			else return dpv[x] + dpv[x+1] + dcr[x+1] + dat[y+1][x] + dat[y+1][x+1];
		}}
		else {
			if (x) {
				if (x == w - 1) [[unlikely]] return dcr[x-1] + dat[1][x-1] + dat[1][x];
				else [[likely]] return dcr[x-1] + dcr[x+1] + dat[1][x-1] + dat[1][x] + dat[1][x+1];
			} else [[unlikely]] return dcr[x+1] + dat[1][x] + dat[1][x+1];
		}
	}
	i32 torusCount(i32 x, i32 y) {
		bool* dnx{ y + 1 == h ? dst : dat[y+1] };
		i32 xpv{ x ? x - 1 : w - 1 },
		    xnx{ x + 1 == w ? 0 : x + 1 };
		return dpv[xpv] + dpv[x] + dpv[xnx]
		     + dcr[xpv]          + dcr[xnx]
		     + dnx[xpv] + dnx[x] + dnx[xnx];
	}
	i32 shTorusCount(i32 x, i32 y) {
		if (vsh) {
			// add column arrays
		}
		i32 xpv{ x ? x - 1 : w - 1 },
		    xnx{ x + 1 == w ? 0 : x + 1 };
		if (y) {if (y + 1 == h) {
			i32 xew{ x - esh + w };
			return dpv[xpv] + dpv[x] + dpv[xnx] + dcr[xpv] + dcr[xnx]
			     + dst[(xew-1) % w] + dst[xew % w] + dst[(xew+1) % w];
		} else
			return dpv[xpv] + dpv[x] + dpv[xnx] + dcr[xpv] + dcr[xnx]
			     + dat[y+1][xpv] + dat[y+1][x] + dat[y+1][xnx];
		} else {
			i32 xew{ x + esh + w };
			return dpv[(xew-1) % w] + dpv[xew % w] + dpv[(xew+1) % w]
			     + dcr[xpv] + dcr[xnx] + dat[y+1][xpv] + dat[y+1][x] + dat[y+1][xnx];
		}
	}
	i32 crossesCount(i32 x, i32 y) { return torusCount(x, y); } // horz. bottle, vert. bottle, and cross-surface
	i32 shCrossesCount(i32 x, i32 y) { return shTorusCount(x, y); }
	i32 sphereCount(i32 x, i32 y) { return torusCount(x, y); }
	i32 shSphereCount(i32 x, i32 y) { return shTorusCount(x, y); } // shift can work with spheres and crosses,
	void planeSet(i32 x, i32 y, bool state) {                      //   but it's messy
		dat[y][x] = state;
		if (y) {if (y == h - 1) {
			bool* anp{ act[y - 1] }; // does this make it faster or slower?
			if (x) {
				if (x == w - 1) [[unlikely]] anp[x-1] = anp[x] = act[y][x-1] = true;
				else [[likely]] anp[x-1] = anp[x] = anp[x+1] = act[y][x-1] = act[y][x+1] = true;
			} else [[unlikely]] anp[x] = anp[x+1] = act[y][x+1] = true;
		} else {
			if (x) act[y-1][x-1] = act[y-1][x] = act[y][x-1] = act[y+1][x-1] = act[y+1][x] = true;
			else act[y-1][x] = act[y-1][x+1] = act[y][x+1] = act[y+1][x] = act[y+1][x+1] = true;
		}}
		else {
			bool* ann{ act[y + 1] };
			if (x) {
				if (x == w - 1) [[unlikely]] act[y][x-1] = ann[x-1] = ann[x] = true;
				else [[likely]] act[y][x-1] = act[y][x+1] = ann[x-1] = ann[x] = ann[x+1] = true;
			} else [[unlikely]] act[y][x+1] = ann[x] = ann[x+1] = true;
		}
	}
	void planeSetFl(i32 x, i32 y, bool state) {
		dat[y][x] = state;
		if (y) {if (y == h - 1) {
			bool* anp{ act[y - 1] }, * anc{ act[y] }; // does this make it faster or slower?
			if (x) {                                 // how computationally expensive is subtracting 1?
				if (x == w - 1) [[unlikely]] anp[x-1] = anp[x] = anc[x-1] = anc[x] = true;
				else [[likely]] anp[x-1] = anp[x] = anp[x+1] = anc[x-1] = anc[x] = anc[x+1] = true;
			} else [[unlikely]] anp[x] = anp[x+1] = anc[x] = anc[x+1] = true;
		} else {
			if (x) act[y-1][x-1] = act[y-1][x] = act[y][x-1] = act[y][x] = act[y+1][x-1] = act[y+1][x] = true;
			else act[y-1][x] = act[y-1][x+1] = act[y][x] = act[y][x+1] = act[y+1][x] = act[y+1][x+1] = true;
		}}
		else {
			bool* anc{ act[y] }, * ann{ act[y + 1] };
			if (x) {
				if (x == w - 1) [[unlikely]] anc[x-1] = anc[x] = ann[x-1] = ann[x] = true;
				else [[likely]] anc[x-1] = anc[x] = anc[x+1] = ann[x-1] = ann[x] = ann[x+1] = true;
			} else [[unlikely]] anc[x] = anc[x+1] = ann[x] = ann[x+1] = true;
		}
	}
	void torusSet(i32 x, i32 y, bool state) {
		dat[y][x] = state;
		bool* anp{ y ? act[y-1] : act[h-1] },
		    * anc{ act[y] },
		    * ann{ y + 1 == h ? *act : act[y+1] };
		i32 xpv{ x ? x - 1 : w - 1 },
		    xnx{ x + 1 == w ? 0 : x + 1 };
		anp[xpv] = anp[x] = anp[xnx] =
		anc[xpv]          = anc[xnx] =
		ann[xpv] = ann[x] = ann[xnx] = true;
	}
	void torusSetFl(i32 x, i32 y, bool state) {
		dat[y][x] = state;
		bool* anp{ y ? act[y-1] : act[h-1] },
		    * anc{ act[y] },
		    * ann{ y + 1 == h ? *act : act[y+1] };
		i32 xpv{ x ? x - 1 : w - 1 },
		    xnx{ x + 1 == w ? 0 : x + 1 };
		anp[xpv] = anp[x] = anp[xnx] =
		anc[xpv] = anc[x] = anc[xnx] =
		ann[xpv] = ann[x] = ann[xnx] = true;
	}
	void shTorusSet(i32 x, i32 y, bool state) {
		dat[y][x] = state;
		i32 xpv{ x ? x - 1 : w - 1 },
		    xnx{ x + 1 == w ? 0 : x + 1 };
		if (vsh) {
			// why am i allowing vertical shift... i regret this
		}
		bool* anc{ act[y] }; // does this make it faster or slower? should i switch it to a #define macro?
		if (y) {if (y + 1 == h) {
			bool* anp{ act[y - 1] }, * ann{ *act };
			i32 xew{ x - esh + w };
			anp[xpv] = anp[x] = anp[xnx] = anc[xpv] = anc[xnx]
			= ann[(xew-1) % w] = ann[xew % w] = ann[(xew+1) % w] = true;
		} else {
			bool* anp{ act[y - 1] }, * ann{ act[y + 1] };
			anp[xpv] = anp[x] = anp[xnx] = anc[xpv] = anc[xnx]
			= ann[xpv] = ann[x] = ann[xnx] = true;
		}} else {
			bool* anp{ act[h - 1] }, * ann{ act[y + 1] };
			i32 xew{ x + esh + w };
			anp[(xew-1) % w] = anp[xew % w] = anp[(xew+1) % w]
			= anc[xpv] = anc[xnx] = ann[xpv] = ann[x] = ann[xnx] = true;
		}
	}
	void shTorusSetFl(i32 x, i32 y, bool state) {
		dat[y][x] = state;
		i32 xpv{ x ? x - 1 : w - 1 },
		    xnx{ x + 1 == w ? 0 : x + 1 };
		if (vsh) {
			//
		}
		bool* anc{ act[y] };
		if (y) {if (y + 1 == h) {
			bool* anp{ act[y - 1] }, * ann{ *act };
			i32 xew{ x - esh + w };
			anp[xpv] = anp[x] = anp[xnx] = anc[xpv] = anc[x] = anc[xnx]
			= ann[(xew-1) % w] = ann[xew % w] = ann[(xew+1) % w] = true;
		} else {
			bool* anp{ act[y - 1] }, * ann{ act[y + 1] };
			anp[xpv] = anp[x] = anp[xnx] = anc[xpv] = anc[x] = anc[xnx]
			= ann[xpv] = ann[x] = ann[xnx] = true;
		}} else {
			bool* anp{ act[h - 1] }, * ann{ act[y + 1] };
			i32 xew{ x + esh + w };
			anp[(xew-1) % w] = anp[xew % w] = anp[(xew+1) % w]
			= anc[xpv] = anc[x] = anc[xnx] = ann[xpv] = ann[x] = ann[xnx] = true;
		}
	}
	void crossesSet(i32 x, i32 y, bool state) { torusSet(x, y, state); }
	void crossesSetFl(i32 x, i32 y, bool state) { torusSetFl(x, y, state); }
	void shCrossesSet(i32 x, i32 y, bool state) { shTorusSet(x, y, state); }
	void shCrossesSetFl(i32 x, i32 y, bool state) { shTorusSetFl(x, y, state); }
	void sphereSet(i32 x, i32 y, bool state) { torusSet(x, y, state); }
	void sphereSetFl(i32 x, i32 y, bool state) { torusSetFl(x, y, state); }
	void shSphereSet(i32 x, i32 y, bool state) { shTorusSet(x, y, state); }
	void shSphereSetFl(i32 x, i32 y, bool state) { shTorusSetFl(x, y, state); }
	void primeEdge() {
		for (i32 i{}; i != w; ++i) act[0][i] = act[h-1][i] = true;
		for (i32 i{}; i != h; ++i) act[i][0] = act[i][w-1] = true;
	}
	void construct(i32 wdt, i32 hgt) {
		assert(wdt >= 0 && wdt < 65536i32 && hgt >= 0 && hgt < 65536i32
		       && (edge != sphere || wdt == hgt)); // if topology is sphere, width must equal height
		if (wdt && hgt) {
			dat = new bool*[hgt];
			act = new bool*[hgt];
			for (i32 i{}, j; i != hgt; ++i) {
				dat[i] = new bool[wdt];
				act[i] = new bool[wdt];
				for (j = 0; j != wdt; ++j) dat[i][j] = false;
			}
		}
		else dat = act = nullptr;
	}
	void moveConstruct(CellGrid&& grid) {
		dat = grid.dat;
		act = grid.act;
		dst = grid.dst;
		dpv = grid.dpv;
		dcr = grid.dcr;
		acr = grid.acr;
		anx = grid.anx;
		dlf = grid.dlf;
		drg = grid.drg;
		als = grid.als;
		pRow = grid.pRow;
		started = grid.started;
		grid.dat = grid.act = nullptr;
		grid.dst = grid.dpv = grid.dcr =
		grid.acr = grid.anx = grid.als =
		grid.dlf = grid.drg = nullptr;
		grid.pRow = nullptr;
		grid.w = grid.h = grid.started = false;
		grid.gen = 0;
	}
public:
	class Rule {
	protected:
		friend CellGrid;
		CellGrid* const grid;
		u32 code;
		mutable char* srep{nullptr};
		bool bArr[9], dArr[9];
		Rule(CellGrid* gd) noexcept : grid{ gd } {} // Use sparingly
		Rule(CellGrid* gd, u32 cd) noexcept
		: grid{ gd }, code{ cd } {
			if (!code) code = defaultRule; // initialization with 0 defaults to Life
			                               // (this is not the case for assignment)
			if (code == -1) code = 131072; // rule 0 "B/S" stored as 2^17
			u32 pos{1};
			bArr[0] = false;
			for (int i{1}; i != 9; ++i, pos <<= 1) bArr[i] = code & pos;
			for (int i{}; i != 9; ++i, pos <<= 1) dArr[i] = ~code & pos;
		}
		Rule(CellGrid* gd, const Rule& rule)
		: grid{ gd }, code{ rule.code } {
			for (int i{}; i != 9; ++i) bArr[i] = rule.bArr[i];
			for (int i{}; i != 9; ++i) dArr[i] = rule.dArr[i];
		}
		Rule(CellGrid* gd, Rule&& rule) noexcept
		: grid{ gd }, code{ rule.code } {
			for (int i{}; i != 9; ++i) bArr[i] = rule.bArr[i]; // these must be copied because they
			for (int i{}; i != 9; ++i) dArr[i] = rule.dArr[i]; //  are not dynamically allocated :(
			if (rule.srep) {	                          //  (more dynamic allocation will be used
				srep = rule.srep;                         // when more families of rules are added)
				rule.srep = nullptr;
			}
		}
		Rule(CellGrid* gd, const char* rulestr)	: grid{ gd } { operator=(rulestr); }
		Rule(CellGrid* gd, stv rulestr)	: grid{ gd } { operator=(rulestr.data()); }
	public:
		explicit Rule(u32 cd) noexcept : Rule{ nullptr, cd } {}
		Rule(const Rule& rule) noexcept : Rule{ nullptr, rule } {}
		Rule(Rule&& rule) noexcept : Rule{ nullptr, std::move(rule) } {}
		explicit Rule(const char* rulestr) : grid{ nullptr } { operator=(rulestr); }
		explicit Rule(stv rulestr) : grid{ nullptr } { operator=(rulestr.data()); }
		virtual ~Rule() { delete[] srep; }
		u32 operator=(u32 cd) noexcept {
			if (!cd) cd = 131072;
			if (code != cd) {
				code = cd;
				u32 pos{1};
				for (int i{1}; i != 9; ++i, pos <<= 1) bArr[i] = code & pos;
				for (int i{}; i != 9; ++i, pos <<= 1) dArr[i] = ~code & pos;
				if (srep) {
					delete[] srep;
					srep = nullptr;
				}
				if (grid && grid->started) for (i32 i{}, j; i != grid->h; ++i)
					for (j = 0; j != grid->w; ++j) grid->act[i][j] = true;
			}
			return cd;
		}
		const Rule& operator=(const Rule& rule) {
			// Self-assignment implicitly handled
			operator=(rule.code);
			return rule;
		}
		Rule& operator=(Rule&& rule) noexcept {
			if (rule.code != code) {
				code = rule.code;
				for (int i{1}; i != 9; ++i) bArr[i] = rule.bArr[i];
				for (int i{0}; i != 9; ++i) dArr[i] = rule.dArr[i];
				if (grid && grid->started) for (i32 i{}, j; i != grid->h; ++i)
					for (j = 0; j != grid->w; ++j) grid->act[i][j] = true;
				delete[] srep;
				if (rule.srep) {
					srep = rule.srep;
					rule.srep = nullptr;
				}
				else srep = nullptr;
			}
			return *this;
		}
		bool readstr(const char* string) {
			if (string == nullptr || !*string) {
				operator=(defaultRule);
				return true;
			}
			if (*string != 'b' && *string != 'B') return false;
			u32 newCode{}, pos;
			signed char stage{}, num;
			for (int i{1}; i != 256; ++i)
				switch (string[i]) {
				case '\0':
					if (stage == 2) {
						if (!newCode) newCode = 131072;
						code = newCode;
						pos = 1;
						for (int i{1}; i != 9; ++i, pos <<= 1) bArr[i] = code & pos;
						for (int i{}; i != 9; ++i, pos <<= 1) dArr[i] = ~code & pos;
						if (srep) {
							delete[] srep;
							srep = nullptr;
						}
						if (grid && grid->started) for (i32 i{}, j; i != grid->h; ++i)
							for (j = 0; j != grid->w; ++j) grid->act[i][j] = true;
						return true;
					}
					return false;
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8':
					if (stage == 1) return false;
					num = string[i] - 48;
					if (!(stage || num)) return false;
					pos = stage ? 512 : 1;
					for (int i{}; i != num; ++i) pos <<= 1;
					newCode |= pos >> 1;
					continue;
				case '/':
					if (stage) return false;
					stage = 1;
					continue;
				case 's': case 'S':
					if (stage == 1 || stage == 0) {
						stage = 2;
						continue;
					}
					return false;
				default:
					return false;
				}
			return false;
		}
		bool readstr(stv string) { return readstr(string.data()); } // todo: use a safer alternative to .data()
		Rule& operator=(const char* string)
			{ if (!readstr(string)) throw std::invalid_argument{"Invalid rulestring"}; return *this; }
		Rule& operator=(stv string)
			{ if (!readstr(string.data())) throw std::invalid_argument{"Invalid rulestring"}; return *this; }
		bool operator==(u32 cd) const { return code == cd; }
		bool operator==(Rule& rl) const { return code == rl.code; }
		bool operator==(const char* string) const { return code == Rule{nullptr, string}.code; }
		bool operator==(stv string) const { return code == Rule{nullptr, string.data()}.code; }
		bool operator!=(u32 cd) const { return code != cd; }
		bool operator!=(Rule& rl) const { return code != rl.code; }
		bool operator!=(const char* string) const { return code != Rule{nullptr, string}.code; }
		bool operator!=(stv string) const { return code != Rule{nullptr, string.data()}.code; }
		bool b(int i) const { return bArr[i]; }
		bool s(int i) const { return !dArr[i]; }
		bool d(int i) const { return dArr[i]; }
		const bool* birthArr() const { return bArr; }
		const bool* deathArr() const { return dArr; }
		bool canFlicker() const {
			for (int i{}; i != 9; ++i) if (bArr[i] && dArr[i]) return true;
			return false;
		}
		const char* cstr() const {
			if (!srep) {
				int i{}, count{4};
				u32 pos{1};
				for (; pos != 131072i32; pos <<= 1) count += code & pos;
				srep = new char[count] {'B'};
				count = 0;
				for (pos = 1; pos != 256; ++i, pos <<= 1)
					if (code & pos) srep[++count] = '1' + i;
				srep[++count] = '/';
				srep[++count] = 'S';
				for (i = 0; pos != 131072i32; ++i, pos <<= 1)
					if (code & pos) srep[++count] = '0' + i;
				srep[++count] = '\0';
			}
			return srep;
		}
		std::string str() const { return cstr(); }
		std::string_view strv() const { return cstr(); }
		operator const char*() const { return cstr(); }
		operator std::string() const { return cstr(); }
		operator std::string_view() const { return cstr(); }
		operator u32() const { return code; }
		friend void swap(CellGrid&, CellGrid&) noexcept;
		friend void swap(Rule&, Rule&) noexcept;
		friend std::istream& operator>>(std::istream&, Rule&);
	};
	class Topology { // This is a class instead of an enumeration to resolve ambiguities in the CellGrid constructors
	protected:
		friend CellGrid;
		CellGrid* const grid;
		char id;
		Topology(CellGrid* parent) : grid{parent} {} // Use sparingly
		Topology(CellGrid* parent, const Topology& top) : grid{parent}, id{top.id} {}
		Topology(CellGrid* parent, char code) : grid{parent}, id{code >= '0' ? code - '0' : code}
			{ assert(id >= 0 && id < 6); }
		Topology(CellGrid* parent, int code) : grid{parent}, id{(char)code} { assert(code >= 0 && code < 6); }
	public:
		static constexpr char names[5][14]{ "plane\0\0\0\0\0\0\0\0", "torus\0\0\0\0\0\0\0\0",
			"Klein bottle\0", "cross-surface", "sphere\0\0\0\0\0\0\0" }; // the compiler complains if i omit the \0's
		explicit Topology(char code) : grid{nullptr}, id{code} { assert(code >= 0 && code < 6); }
		Topology(const Topology& top) : grid{nullptr}, id{top.id} {}
		void operator=(char code) {
			if (code >= '0') code -= '0';
			assert(code >= 0 && code < 6);
			id = code;
			if (grid && grid->started) grid->primeEdge();
		}
		void operator=(int code) {
			assert(code >= 0 && code < 6);
			id = code;
			if (grid && grid->started) grid->primeEdge();
		}
		void operator=(const Topology& ebh) {
			id = ebh.id;
			if (grid && grid->started) grid->primeEdge();
		}
		bool operator==(const Topology& ebh) const { return id == ebh.id; }
		bool operator!=(const Topology& ebh) const { return id != ebh.id; }
		bool operator==(char code) const { return id == code || id - '0' == code; }
		bool operator!=(char code) const { return id != code && id - '0' != code; }
		bool operator==(int code) const { return (int)id == code; }
		bool operator!=(int code) const { return (int)id != code; }
		char getCode() const { return id; }
		const char* cstr() const { return names[id == 4 ? 3 : id == 5 ? 4 : id]; }
		std::string str() const { return names[id == 4 ? 3 : id == 5 ? 4 : id]; }
		std::string_view strv() const { return names[id == 4 ? 3 : id == 5 ? 4 : id]; }
		operator const char*() const { return names[id == 4 ? 3 : id == 5 ? 4 : id]; }
		operator std::string() const { return names[id == 4 ? 3 : id == 5 ? 4 : id]; }
		operator std::string_view() const { return names[id == 4 ? 3 : id == 5 ? 4 : id]; }
		friend void swap(CellGrid&, CellGrid&) noexcept;
		friend void swap(Topology&, Topology&) noexcept;
		friend std::istream& operator>>(std::istream&, Topology&);
	};
	i32 width() { return w; };
	i32 height() { return h; };
	Rule rule;
	static const Topology plane, torus, bottle, vbottle, hbottle, cross, sphere;
	/* these are initialized outside of the class (right before the end of the file)
	   with the values 0, 1, 2, 2, 3, 4, 5, respectively */
	Topology edge;
	Topology& topology{edge};
	i32 gen;
	CellGrid(i32 wdt, i32 hgt, u32 rl, Topology ebh = plane, i32 gn = 0)
	  : w{wdt}, h{hgt}, edge{this, ebh.id}, gen{gn}, rule{this, rl}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, const char* rl, Topology ebh = plane, i32 gn = 0)
	  : w{wdt}, h{hgt}, edge{this, ebh.id}, gen{gn}, rule{this, rl}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, stv rl, Topology ebh = plane, i32 gn = 0)
	  : w{wdt}, h{hgt}, edge{this, ebh.id}, gen{gn}, rule{this, rl}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, const Rule& rl, Topology ebh = plane, i32 gn = 0)
	  : w{wdt}, h{hgt}, edge{this, ebh.id}, gen{gn}, rule{this, rl}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, Rule&& rl, Topology ebh = plane, i32 gn = 0)
	  : w{wdt}, h{hgt}, edge{this, ebh.id}, gen{gn}, rule{this, std::move(rl)}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, Topology ebh = plane) : w{wdt}, h{hgt}, edge{this, ebh.id}, gen{}, rule{this, 0u}
		{ construct(wdt, hgt); }
	CellGrid(const CellGrid& grid, const char* rl, Topology ebh, i32 gn) : CellGrid(grid.w, grid.h, rl, ebh, gn)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, const char* rl, Topology ebh) : CellGrid(grid.w, grid.h, rl, ebh, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, const char* rl) : CellGrid(grid.w, grid.h, rl, grid.edge, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, stv rl, Topology ebh, i32 gn) : CellGrid(grid.w, grid.h, rl, ebh, gn)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, stv rl, Topology ebh) : CellGrid(grid.w, grid.h, rl, ebh, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, stv rl) : CellGrid(grid.w, grid.h, rl, grid.edge, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, const Rule& rl, Topology ebh, i32 gn) : CellGrid(grid.w, grid.h, rl, ebh, gn)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, const Rule& rl, Topology ebh) : CellGrid(grid.w, grid.h, rl, ebh, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, const Rule& rl) : CellGrid(grid.w, grid.h, rl, grid.edge, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, Rule&& rl, Topology ebh, i32 gn) : CellGrid(grid.w, grid.h, std::move(rl), ebh, gn)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, Rule&& rl, Topology ebh) : CellGrid(grid.w, grid.h, std::move(rl), ebh, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, Rule&& rl) : CellGrid(grid.w, grid.h, std::move(rl), grid.edge, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, u32 rl, Topology ebh, i32 gn) : CellGrid(grid.w, grid.h, rl, ebh, gn)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, u32 rl, Topology ebh) : CellGrid(grid.w, grid.h, rl, ebh, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, u32 rl) : CellGrid(grid.w, grid.h, rl, grid.edge, grid.gen)
		{ for (i32 i{}, j; i != grid.h; ++i) for (j = 0; j != grid.w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid) : CellGrid(grid, grid.rule, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, const char* rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		rule = rl;
		edge = ebh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, const char* rl, Topology ebh) : CellGrid(std::move(grid), rl, ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, const char* rl) : CellGrid(std::move(grid), rl, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, stv rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		rule = rl;
		edge = ebh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, stv rl, Topology ebh) : CellGrid(std::move(grid), rl, ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, stv rl) : CellGrid(std::move(grid), rl, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, const Rule& rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		rule = rl;
		edge = ebh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, const Rule& rl, Topology ebh) : CellGrid(std::move(grid), rl, ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, const Rule& rl) : CellGrid(std::move(grid), rl, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, Rule&& rl, Topology ebh, i32 gn) noexcept : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		rule = std::move(rl);
		edge = ebh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, Rule&& rl, Topology ebh) : CellGrid(std::move(grid), std::move(rl), ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, Rule&& rl) : CellGrid(std::move(grid), std::move(rl), grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, u32 rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		rule = rl;
		edge = ebh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, u32 rl, Topology ebh) : CellGrid(std::move(grid), rl, ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, u32 rl) : CellGrid(std::move(grid), rl, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid) noexcept : CellGrid(std::move(grid), std::move(grid.rule), grid.edge, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const char* rl, Topology ebh, i32 gn)
	: CellGrid(wdt, hgt, rl, ebh, gn) { for (i32 i{}, j; i != grid.h && i != h; ++i)
											for (j = 0; j != grid.w && j != w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const char* rl, Topology ebh)
	: CellGrid(grid, wdt, hgt, rl, ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const char* rl)
	: CellGrid(grid, wdt, hgt, rl, grid.edge, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, stv rl, Topology ebh, i32 gn) : CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != grid.h && i != h; ++i)
			  for (j = 0; j != grid.w && j != w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, stv rl, Topology ebh)
	: CellGrid(grid, wdt, hgt, rl, ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, stv rl) : CellGrid(grid, wdt, hgt, rl, grid.edge, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const Rule& rl, Topology ebh, i32 gn)
	: CellGrid(wdt, hgt, rl, ebh, gn) { for (i32 i{}, j; i != grid.h && i != h; ++i)
		                                    for (j = 0; j != grid.w && j != w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const Rule& rl, Topology ebh)
	: CellGrid(grid, wdt, hgt, rl, ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const Rule& rl)
	: CellGrid(grid, wdt, hgt, rl, grid.edge, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, Rule&& rl, Topology ebh, i32 gn)
	: CellGrid(wdt, hgt, std::move(rl), ebh, gn)
		{ for (i32 i{}, j; i != grid.h && i != h; ++i)
			  for (j = 0; j != grid.w && j != w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, Rule&& rl, Topology ebh)
	: CellGrid(grid, wdt, hgt, std::move(rl), ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, Rule&& rl)
	: CellGrid(grid, wdt, hgt, std::move(rl), grid.edge, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, u32 rl, Topology ebh, i32 gn) : CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != grid.h && i != h; ++i)
			  for (j = 0; j != grid.w && j != w; ++j) dat[i][j] = grid.dat[i][j]; }
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, u32 rl, Topology ebh)
	: CellGrid(grid, wdt, hgt, rl, ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, u32 rl) : CellGrid(grid, wdt, hgt, rl, grid.edge, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt) : CellGrid(grid, wdt, hgt, grid.rule, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const char* rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		if (started) {
			started = false;
			rule = rl;
			started = true;
		} else rule = rl;
		edge = ebh;
		gen = gn;
		resize(wdt, hgt);
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const char* rl, Topology ebh)
	: CellGrid(std::move(grid), wdt, hgt, rl, ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const char* rl)
	: CellGrid(std::move(grid), wdt, hgt, rl, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, stv rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		if (started) {
			started = false;
			rule = rl;
			started = true;
		} else rule = rl;
		edge = ebh;
		gen = gn;
		resize(wdt, hgt);
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, stv rl, Topology ebh)
	: CellGrid(std::move(grid), wdt, hgt, rl, ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, stv rl)
	: CellGrid(std::move(grid), wdt, hgt, rl, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const Rule& rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		if (started) {
			started = false;
			rule = rl;
			started = true;
		} else rule = rl;
		edge = ebh;
		gen = gn;
		resize(wdt, hgt);
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const Rule& rl, Topology ebh)
	: CellGrid(std::move(grid), wdt, hgt, rl, ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const Rule& rl)
	: CellGrid(std::move(grid), wdt, hgt, rl, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, Rule&& rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		if (started) {
			started = false;
			rule = std::move(rl);
			started = true;
		} else rule = std::move(rl);
		edge = ebh;
		gen = gn;
		resize(wdt, hgt);
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, Rule&& rl, Topology ebh)
	: CellGrid(std::move(grid), wdt, hgt, std::move(rl), ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, Rule&& rl)
	: CellGrid(std::move(grid), wdt, hgt, std::move(rl), grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, u32 rl, Topology ebh, i32 gn) : rule{this}, edge{this} {
		moveConstruct(std::move(grid));
		if (started) {
			started = false;
			rule = rl;
			started = true;
		} else rule = rl;
		gen = gn;
		resize(wdt, hgt);
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, u32 rl, Topology ebh)
	: CellGrid(std::move(grid), wdt, hgt, rl, ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, u32 rl)
	: CellGrid(std::move(grid), wdt, hgt, rl, grid.edge, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt)
	: CellGrid(std::move(grid), wdt, hgt, std::move(grid.rule), grid.edge, grid.gen) {}
		// why does the debugger warn me about "use of a moved-from object"? grid.rule hasn't been moved yet...
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, const char* rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, stv rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, const Rule& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, Rule&& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, std::move(rl), ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, u32 rl = 0, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* cells, i32 wdt, i32 hgt, const char* rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	CellGrid(const bool* cells, i32 wdt, i32 hgt, stv rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	CellGrid(const bool* cells, i32 wdt, i32 hgt, const Rule& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	CellGrid(const bool* cells, i32 wdt, i32 hgt, Rule&& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, std::move(rl), ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	CellGrid(const bool* cells, i32 wdt, i32 hgt, u32 rl = 0, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, const char* rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) { dat = cells; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, stv rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) { dat = cells; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, Rule& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) { dat = cells; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, Rule&& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, std::move(rl), ebh, gn) { dat = cells; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, u32 rl = 0, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) { dat = cells; }
	template<size_t W, size_t H>
	CellGrid(const bool cells[H][W], i32 wdt, i32 hgt, const char* rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) { assert(wdt == W && hgt == H);
		for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<size_t W, size_t H>
	CellGrid(const bool cells[H][W], i32 wdt, i32 hgt, stv rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) { assert(wdt == W && hgt == H);
		for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<size_t W, size_t H>
	CellGrid(const bool cells[H][W], i32 wdt, i32 hgt, const Rule& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) { assert(wdt == W && hgt == H);
		for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<size_t W, size_t H>
	CellGrid(const bool cells[H][W], i32 wdt, i32 hgt, Rule&& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, std::move(rl), ebh, gn) { assert(wdt == W && hgt == H);
		for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<size_t W, size_t H>
	CellGrid(const bool cells[H][W], i32 wdt, i32 hgt, u32 rl = 0, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) { assert(wdt == W && hgt == H);
		for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	static constexpr auto intlim{ std::numeric_limits<int>::max() };
	template<size_t W, size_t H> CellGrid(const bool cells[H][W], const char* rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(W, H, rl, ebh, gn) { assert(W <= intlim && H <= intlim);
		for (i32 i{}, j; i != H; ++i) for (j = 0; j != W; ++j) dat[i][j] = cells[i][j]; }
	template<size_t W, size_t H> CellGrid(const bool cells[H][W], stv rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(W, H, rl, ebh, gn) { assert(W <= intlim && H <= intlim);
		for (i32 i{}, j; i != H; ++i) for (j = 0; j != W; ++j) dat[i][j] = cells[i][j]; }
	template<size_t W, size_t H> CellGrid(const bool cells[H][W], const Rule& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(W, H, rl, ebh, gn) { assert(W <= intlim && H <= intlim);
		for (i32 i{}, j; i != H; ++i) for (j = 0; j != W; ++j) dat[i][j] = cells[i][j]; }
	template<size_t W, size_t H> CellGrid(const bool cells[H][W], Rule&& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(W, H, std::move(rl), ebh, gn) { assert(W <= intlim && H <= intlim);
		for (i32 i{}, j; i != H; ++i) for (j = 0; j != W; ++j) dat[i][j] = cells[i][j]; }
	template<size_t W, size_t H> CellGrid(const bool cells[H][W], u32 rl = 0, Topology ebh = plane, i32 gn = 0)
	: CellGrid(W, H, rl, ebh, gn) { assert(W <= intlim && H <= intlim);
		for (i32 i{}, j; i != H; ++i) for (j = 0; j != W; ++j) dat[i][j] = cells[i][j]; }
	template<size_t S> CellGrid(const bool cells[S], i32 wdt, i32 hgt, const char* rl, Topology ebh = plane, i32 gn= 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	template<size_t S> CellGrid(const bool cells[S], i32 wdt, i32 hgt, stv rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	template<size_t S> CellGrid(const bool cells[S], i32 wdt, i32 hgt, const Rule& rl, Topology ebh = plane, i32 gn= 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	template<size_t S> CellGrid(const bool cells[S], i32 wdt, i32 hgt, Rule&& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, std::move(rl), ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	template<size_t S> CellGrid(const bool cells[S], i32 wdt, i32 hgt, u32 rl = 0, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i * wdt + j]; }
	/*
	// I need to somehow prevent <T> from matching int, bool**, bool**&&, CellGrid&, and CellGrid&&
	template<class T> CellGrid(T& cells, i32 wdt, i32 hgt, const char* rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) {for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> CellGrid(T& cells, i32 wdt, i32 hgt, stv rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) {for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> CellGrid(T& cells, i32 wdt, i32 hgt, const Rule& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, rl, ebh, gn) {for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<class T> CellGrid(T& cells, i32 wdt, i32 hgt, Rule&& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(wdt, hgt, std::move(rl), ebh, gn)
		{ for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> CellGrid(T& cells, i32 wdt, i32 hgt, u32 rl, Topology ebh = plane, i32 gn = 0
	: CellGrid(wdt, hgt, rl, ebh, gn) {for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> CellGrid(T& cells, const char* rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(cells[0].size(), cells.size(), rl, ebh, gn) 
		{ for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> CellGrid(T& cells, stv rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(cells[0].size(), cells.size(), rl, ebh, gn)
		{ for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> CellGrid(T& cells, const Rule& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(cells[0].size(), cells.size(), rl, ebh, gn)
		{ for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> CellGrid(T& cells, Rule&& rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(cells[0].size(), cells.size(), std::move(rl), ebh, gn)
		{ for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> CellGrid(T& cells, i32 rl, Topology ebh = plane, i32 gn = 0)
	: CellGrid(cells[0].size(), cells.size(), rl, ebh, gn)
		{ for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	template<class T> explicit CellGrid(T& cells) : CellGrid((*cells).size(), cells.size())
		{ for (i32 x, y{}; auto i : cells) for (x = 0, ++y; auto j : *i) dat[y][x] = *j; }
	*/
	// am i crazy or do classes usually have this many constructors?
	// is there a better way to do this??

	virtual ~CellGrid() {
		for (i32 i{}; i != h; ++i) {
			delete[] dat[i];
			delete[] act[i];
		}
		delete[] dat;
		delete[] act;
		delete[] dst;
		delete[] dpv;
		delete[] dcr;
		delete[] acr;
		delete[] anx;
		delete[] als;
		delete[] dlf;
		delete[] drg;
		delete[] pRow;
	}
	CellGrid& operator=(CellGrid& grid) {
		if (&grid == this) return *this;
		for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) dat[i][j] = grid.dat[i][j];
		if (started) {
			if (grid.started) {
				for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) act[i][j] = grid.act[i][j];
				started = false;
				rule = grid.rule;
				started = true;
			}
			else {
				for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) act[i][j] = true;
				started = false;
				rule = grid.rule;
			}
		}
		else {
			rule = grid.rule;
			if (grid.started) {
				for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) act[i][j] = grid.act[i][j];
				started = true;
			}
		}
		gen = grid.gen;
		return *this;
	}
	CellGrid& operator=(CellGrid&& grid) noexcept {
		if (&grid == this) return *this;
		dat = grid.dat;
		act = grid.act;
		dst = grid.dst;
		dpv = grid.dpv;
		dcr = grid.dcr;
		acr = grid.acr;
		anx = grid.anx;
		als = grid.als;
		dlf = grid.dlf;
		drg = grid.drg;
		pRow = grid.pRow;
		w = grid.w;
		h = grid.h;
		rule = std::move(grid.rule);
		started = grid.started;
		gen = grid.gen;
		grid.dat = grid.act = nullptr;
		grid.dst = grid.dpv = grid.dcr =
		grid.acr = grid.anx = grid.als =
		grid.dlf = grid.drg = nullptr;
		grid.w = grid.h = 0;
		started = grid.started = false;
		grid.gen = 0;
		return *this;
	}
	CellGrid& operator=(const bool* const* grid) { // make sure input grid has the same dimensions as CellGrid!
		for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j)
			dat[i][j] = grid[i][j],
			act[i][j] = true;
		started = false;
		return *this;
	}
	CellGrid& operator=(bool**&& grid) noexcept { // especially here!!! the error will not surface immediately
		if (grid == dat) return *this;
		for (i32 i{}; i != h; ++i) delete[] dat[i];
		delete[] dat;
		dat = grid;
		for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) act[i][j] = true;
		started = false;
		return *this;
	}
	void shift(i32 shft) {
		if (started) primeEdge();
		if (vsh) esh = shft % h; // C++'s modulo % operator treats the dividend as an absolute
		else esh = shft % w;     //   value and then multiplies by the sign of the dividend
	}
	void shift(i32 shft, bool vshft) {
		if (started) primeEdge();
		if (vshft) {
			esh = shft % h;
			vsh = true;
		} else {
			esh = shft % w;
			vsh = false;
		}
	}
	const bool* const* data() { return dat; }
	const bool* const* active() const { return act; } // for debugging | warning: act may be uninitialized
	bool operator()(i32 x, i32 y) const {
		if (x < 0 || x >= w || y < 0 || w >= h) return false;
		return dat[y][x];
	}
	bool operator()(i32 x, i32 y, bool value) {
		assert(x >= 0 && x < w && y >= 0 && y < h);
		if (dat[y][x] != value) {
			dat[y][x] = value;
			if (started)
			    act[y-1][x-1] = act[y-1][x] = act[y-1][x+1]
			    = act[y][x-1] = act[ y ][x] = act[y][x+1] =
			    act[y+1][x-1] = act[y+1][x] = act[y+1][x+1]
			= true;
		}
		return value;
	}
	virtual CellGrid& operator++() {
		if (!w || !h) return *this;
		bool flicker{ rule.canFlicker() };
		const i32 wm1{ w - 1 }, hm1{ h - 1 };
		i32 (CellGrid::*edgeCount)(i32, i32); // C++ is confusing
		void (CellGrid::*edgeSet)(i32, i32, bool);
		algprep(edgeCount, edgeSet, flicker);
		algorithm(rule.birthArr(), rule.deathArr(), edgeCount, edgeSet, flicker);
		++gen;
		return *this;
	}
	CellGrid& operator++(int) { return operator++(); }
	virtual CellGrid& operator+=(i32 gens) {
		if (gens < 0) throw std::logic_error{ "The operation you have requested is physically impossible." };
		if (!w || !h) return *this;
		bool flicker{ rule.canFlicker() };
		const bool* birth{ rule.birthArr() };
		const bool* death{ rule.deathArr() };
		const i32 wm1{ w - 1 }, hm1{ h - 1 };
		i32 (CellGrid::*edgeCount)(i32, i32);
		void (CellGrid::*edgeSet)(i32, i32, bool);
		algprep(edgeCount, edgeSet, flicker);
		for (i32 i{}; i != gens; ++i) algorithm(birth, death, edgeCount, edgeSet, flicker);
		gen += gens;
		return *this;
	}
	CellGrid operator+(i32 gens) const {
		CellGrid newGrid{*this};
		return newGrid += gens;
	}
	i64 count(bool state = true) const {
		i64 result{};
		for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) if (dat[i][j]) result++;
		if (state) return result;
		else return static_cast<i64>(w * h) - result;
	}
	void resize(i32 wdt, i32 hgt, bool keepCells = true, bool center = false) {
		assert(wdt >= 0 && hgt >= 0);
		if (wdt && hgt) {
			if (!w || !h) {
				if (!dat) {
					dat = new bool* [hgt];
					act = new bool* [hgt];
				}
				for (i32 i{}; i != h; ++i) {
					dat[i] = new bool[wdt];
					act[i] = new bool[wdt];
				}
				w = wdt;
				h = hgt;
				return;
			}
		}
		else {
			if (dat) {
				for (i32 i{}; i != h; ++i) {
					delete[] dat[i];
					delete[] act[i];
				}
				delete[] dat;
				delete[] act;
			}
			if (!wdt) {
				delete[] dst;
				delete[] dpv;
				delete[] dcr;
				delete[] acr;
				delete[] anx;
				delete[] als;
				dst = dpv = dcr = acr = anx = als = nullptr;
			}
			if (!hgt) {
				delete[] dlf;
				delete[] drg;
				dlf = drg = nullptr;
			}
			w = wdt;
			h = hgt;
			return;
		}
		if (w == wdt) {
			if (h == hgt) {
				if (!keepCells) clear();
				return;
			}
			goto heqhgt;
		}
		else {
			delete[] dst;
			delete[] dpv;
			delete[] dcr;
			delete[] acr;
			delete[] anx;
			delete[] als;
			dst = dpv = dcr = acr = anx = als = nullptr;
		}
		#define hfsb(a, b) ( a < b ? (a - b) / 2 : (a - b + 1) / 2 )
		if (h == hgt) heqhgt: {
			bool* rda, * rac;
			for (i32 i{}, j; i != h; ++i) {
				if (keepCells) {
					rda = new bool[wdt];
					rac = new bool[wdt];
					j = 0;
					if (center) for (i32 k{ hfsb(w, wdt) }; j != wdt; ++j, ++k) rda[j] =
						k >= 0 && k < w ? dat[i][k] : false;
					else for (; j != wdt; ++j) rda[j] = j < w ? dat[i][j] : false;
				} else rda = new bool[wdt]{},
				       rac = new bool[wdt]{};
				delete[] dat[i];
				delete[] act[i];
				dat[i] = rda;
				act[i] = rac;
			}
			w = wdt;
		}
		else {
			bool** nda{ new bool*[hgt] },
			    ** nac{ new bool*[hgt] };
			for (i32 i{}, j; i != hgt; ++i) {
				nda[i] = new bool[wdt];
				nac[i] = new bool[wdt];
				j = 0;
				if (keepCells) {
					if (center) {
						for (i32 hoff{ hfsb(w, wdt) },
						         ysrc{ (i32)i + hfsb(h, hgt) }; j != wdt; ++j)
							if (ysrc >= 0 && ysrc < h && j + hoff >= 0 && j + hoff < w)
								nda[i][j] = dat[ysrc][j + hoff],
								nac[i][j] = act[ysrc][j + hoff];
							else nda[i][j] = nac[i][j] = false;
					}
					else for (; j != wdt; ++j)
						nda[i][j] = dat[i][j],
						nac[i][j] = act[i][j];
				}
				else for (; j != wdt; ++j) nda[i][j] = false, nac[i][j] = false;
				if (i < h)
					delete[] dat[i],
					delete[] act[i];
			}
			if (keepCells) {
				i32 i{};
				if (center) {
					i32 hoff{ (wdt - w) / 2 }, voff{ (hgt - h) / 2 };
					if (w < wdt) i = hoff;
					if (h < hgt) {
						for (; i - hoff != w && i != wdt; ++i) if (i >= 0)
							(voff ? nac[voff-1][i] : nac[hgt-1][i]) = nac[voff][i] // buffer overrun warning?
							  = nac[h-1 + voff][i] = nac[h + voff][i] = true;      // i don't understand how
						i = voff;
					}
					else {
						for (; i - hoff != w && i != wdt; ++i) if (i >= 0)
							nac[0][i] = nac[hgt - 1][i] = true;
						i = 0;
					}
					if (w < wdt) for (; i - voff != h && i != hgt; ++i) if (i >= 0)
						(hoff ? nac[i][hoff-1] : nac[i][wdt-1]) = nac[i][hoff]
						  = nac[i][w-1 + hoff] = nac[i][w + hoff] = true;
					else for (; i - voff != h && i != hgt; ++i) if (i >= 0)
						nac[i][0] = nac[i][wdt-1] = true;
					// fix these                vvv
					//nac[hgt-1][wdt-1] = true;
					//if (w < wdt) nac[hgt-1][w] = (h > hgt) ? true : nac[h][w] = true;
					//if (h < hgt) nac[h][wdt-1] = (w > wdt) ? true : nac[h][w] = true;
				}
				else {
					if (h < hgt) for (; i != w && i != wdt; ++i)
						nac[0][i] = nac[h-1][i] = nac[h][i] = nac[hgt-1][i] = true;
					else for (; i != wdt; ++i) nac[0][i] = nac[hgt-1][i] = true;
					if (w < wdt) for (i = 0; i != h && i != hgt; ++i)
						nac[i][0] = nac[i][w-1] = nac[i][w] = nac[i][wdt-1] = true;
					else for (i = 0; i != hgt; ++i) nac[i][0] = nac[i][wdt-1] = true;
					nac[hgt-1][wdt-1] = true;
					if (w < wdt) nac[hgt-1][w] = (h >= hgt) ? true : nac[h][w] = true;
					if (h < hgt) nac[h][wdt-1] = (w >= wdt) ? true : nac[h][w] = true;
				}
			}
			delete[] dat;
			delete[] act;
			dat = nda;
			act = nac;
			delete[] dlf;
			delete[] drg;
			dlf = drg = nullptr;
			w = wdt;
			h = hgt;
		}
	}
	void resize(bool keepCells, bool center = false) { resize(0, 0, keepCells, center); }
	void clear() {
		started = false;
		for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) dat[i][j] = act[i][j] = false;
	}
	void pause() { started = false; } // [Advanced] The grid resumes whenever operator++ is called.
	     //        Resuming the grid requires some extra calculations, however, so pause only when necessary.
	bool isActive() const { return started; }
	bool isValid() const { return dat; } // returns false if width==0 or height==0 or if the grid has been moved-from
	operator bool() const { return dat; }
	class iterator; // almost the exact same as const_iterator but inheritance doesn't work in this case :(
	class const_iterator {
	protected:
		friend class CellGrid;
		friend class iterator;
		// To fit within 256 bits, this class is optimized for operator* at the cost of a slower operator--
		const CellGrid* grid;
		const bool* pos, * rend; // warning: this iterator breaks when the grid is
		i32 row{};               //    resized, use grid.cbegin() to get a new one
		const_iterator(const CellGrid* gd, const bool* ps, i32 w) : grid{ gd }, pos{ ps }, rend{ ps + w } {}
		// this could be faster if it were larger, would it be worth it? or should iterator classes stay lightweight
	public:		                                                                                      // (spaceship)
		const_iterator(const const_iterator&) = default;
		const_iterator(const iterator& iter) : grid{ iter.grid }, pos{ iter.pos }, rend{ iter.rend }, row{ iter.row } {}
		const_iterator& operator=(const const_iterator& iter) {
			grid = iter.grid, pos = iter.pos, rend = iter.rend, row = iter.row;
			return *this;
		}
		const_iterator& operator=(const iterator& iter) {
			grid = iter.grid, pos = iter.pos, rend = iter.rend, row = iter.row;
			return *this;
		}
		bool operator*() const {
			assert(row >= 0 && row < grid->h); // hopefully this doesn't affect release performance
			return *pos;
		}
		operator const bool*() const = delete; // if i decide to make CellGrid::dat one-dimensional, this
		const_iterator& operator++() {         //  will become available (and everything will run faster)
			++pos;
			if (pos == rend) {
				++row;
				pos = grid->dat[row];
				rend = pos + grid->w;
			}
			return *this;
		}
		const_iterator& operator++(int) { return operator++(); }
		const_iterator& operator+=(i32 offset) { // should i return by value or reference?
			if (offset < 0) return operator-=(offset);
			pos += offset;
			if (pos >= rend) {
				row += offset / grid->w + 1;
				bool* rowp{ grid->dat[row] };
				pos = rowp + (pos - rend) % grid->w;
				rend = rowp + grid->w;
			}
			return *this;
		}
		const_iterator& operator--() {
			if (pos == rend - grid->w) { // would the [[unlikely]] attribute be helpful here?
				--row;
				rend = grid->dat[row] + grid->w;
				pos = rend - 1;
			} else --pos; // does the else branch make it faster or slower?
			return *this;
		}
		const_iterator& operator--(int) { return operator--(); }
		const_iterator& operator-=(i32 offset) {
			if (offset < 0) return operator+=(offset);
			pos -= offset;
			if (pos < rend - grid->w) {
				row -= offset / grid->w - 1; // C++'s integer division rounds towards 0
				i32 pmod{ (i32)(pos - rend) % grid->w }; // C++'s modulo operator keeps the sign of the dividend
				rend = grid->dat[row] + grid->w;
				pos = rend + pmod; // pmod is negative
			}
		}
		const_iterator& nextRow() {
			i32 emp{ static_cast<i32>(pos - rend) }; // negative
			rend = grid->dat[++row] + grid->w;
			pos = rend + emp;
			return *this;
		}
		const_iterator& prevRow() {
			i32 emp{ static_cast<i32>(pos - rend) };
			rend = grid->dat[--row] + grid->w;
			pos = rend + emp;
			return *this;
		}
		const_iterator& moveRow(i32 offset) {
			i32 emp{ static_cast<i32>(pos - rend) };
			rend = grid->dat[row += offset] + grid->w;
			pos = rend + emp;
			return *this;
		}
		bool operator==(const_iterator& iter) const { return pos == iter.pos; }
		bool operator==(iterator& iter) const { return pos == iter.pos; }
		bool operator==(const bool* ptr) const { return pos == ptr; }
	};
	const_iterator cbegin() const { return const_iterator{ this, *dat, w }; }
	const_iterator cend() const { return const_iterator{ this, dat[h-1] + w-1, w }; }
	//const_iterator crbegin() const { return const_iterator{ this, dat[h-1] + w, w }; }
	//const_iterator crend() const { return const_iterator{ this, *dat - 1, w }; }
	class iterator {
		// This iterator can only be used when the grid has not started or is paused
		//  (while it is not keeping track of active cells)
	protected:
		friend class CellGrid;
		friend class const_iterator;
		const CellGrid* grid;
		bool* pos, * rend;
		i32 row{};
		iterator(const CellGrid* gd, bool* ps, i32 w) : grid{gd}, pos{ps}, rend{ ps + w } {}
	public:
		iterator(const iterator&) = default;
		iterator(const const_iterator& iter) : grid{ iter.grid }, pos{ const_cast<bool*>(iter.pos) },
			rend{ const_cast<bool*>(iter.rend) }, row{ iter.row } {} // Is this bad coding practice?
		iterator& operator=(const iterator& iter) {
			grid = iter.grid, pos = const_cast<bool*>(iter.pos), rend = const_cast<bool*>(iter.rend), row = iter.row;
			return *this;
		}
		iterator& operator=(const const_iterator& iter) {
			grid = iter.grid, pos = const_cast<bool*>(iter.pos), rend = const_cast<bool*>(iter.rend), row = iter.row;
			return *this;
		}
		bool& operator*() const {
			assert(!grid->started && row >= 0 && row < grid->h);
			// If this assertion is failing, try pausing the grid using grid.pause() before using this iterator
			return *pos;
		}
		iterator& operator++() {
			++pos;
			if (pos == rend) {
				++row;
				pos = grid->dat[row];
				rend = pos + grid->w;
			}
			return *this;
		}
		iterator& operator++(int) { return operator++(); }
		iterator& operator+=(i32 offset) {
			if (offset < 0) return operator-=(offset);
			pos += offset;
			if (pos >= rend) {
				row += offset / grid->w + 1;
				bool* rowp{ grid->dat[row] };
				pos = rowp + (pos - rend) % grid->w;
				rend = rowp + grid->w;
			}
			return *this;
		}
		iterator& operator--() {
			if (pos == rend - grid->w) {
				--row;
				rend = grid->dat[row] + grid->w;
				pos = rend - 1;
			} else --pos;
			return *this;
		}
		iterator& operator--(int) { return operator--(); }
		iterator& operator-=(i32 offset) {
			if (offset < 0) return operator+=(offset);
			pos -= offset;
			if (pos < rend - grid->w) {
				row -= offset / grid->w - 1; // C++'s integer division rounds towards 0
				i32 pmod{ (i32)(pos - rend) % grid->w }; // C++'s modulo operator keeps the sign of the dividend
				rend = grid->dat[row] + grid->w;
				pos = rend + pmod; // pmod is negative
			}
		}
		iterator& nextRow() {
			i32 emp{ static_cast<i32>(pos - rend) }; // negative
			rend = grid->dat[++row] + grid->w;
			pos = rend + emp;
			return *this;
		}
		iterator& prevRow() {
			i32 emp{ static_cast<i32>(pos - rend) };
			rend = grid->dat[--row] + grid->w;
			pos = rend + emp;
			return *this;
		}
		iterator& moveRow(i32 offset) {
			i32 emp{ static_cast<i32>(pos - rend) };
			rend = grid->dat[row += offset] + grid->w;
			pos = rend + emp;
			return *this;
		}
		operator bool*() const = delete;
		bool operator==(bool* ptr) const { return pos == ptr; }
	};
	iterator begin() const { return iterator{ this, *dat, w }; }
	iterator end() const { return iterator{ this, dat[h-1] + w-1, w }; }
	//iterator rbegin() const { return iterator{ this, dat[h-1] + w, w }; }
	//iterator rend() const { return iterator{ this, *dat - 1, w }; }
	void print(bool pGen = false, std::ostream& ost = std::cout) const {
		bool longpr{};
		if (!pRow) pRow = new uch[w + 2];
		else if (*pRow == '\0') longpr = true;
		if (pGen) ost << "Generation " << gen << '\n';
		for (i32 i{}, j; i < h; i += 2) {
			j = 0;
			if (i + 1 == h) for (; j != w; ++j) pRow[j] = dat[i][j] ? 223 : ' ';
			else for (; j != w; ++j) pRow[j] =
				dat[i][j] ? (dat[i+1][j] ? 219 : 223) : (dat[i+1][j] ? 220 : ' ');
			pRow[w] = '\n';
			pRow[w+1] = '\0';
			ost << pRow;
		}
		if (longpr) *pRow = '\0';
		else *pRow = (uch)255;
	}
	void printSafe(bool pGen = false, uch ch1 = '\0', uch ch2 = '\0', std::ostream& ost = std::cout) const {
		assert(ch1 != (uch)255 && ch2 != (uch)255);
		if (ch1) { if (!ch2) ch2 = ch1; }
		else ch1 = '[', ch2 = ']';
		if (!pRow) pRow = new uch[w * 2 + 2];
		else if (*pRow == 255) {
			delete[] pRow;
			pRow = new uch[w * 2 + 2];
		}
		if (pGen) ost << "Generation " << gen << '\n';
		for (i32 i{}, j; i < h; ++i) {
			for (j = 0; j != w; ++j) {
				pRow[j*2] = dat[i][j] ? ch1 : ' ';
				pRow[j*2+1] = dat[i][j] ? ch2 : ' ';
			}
			pRow[w*2] = '\n';
			pRow[w*2+1] = '\0';
			ost << pRow;
		}
		*pRow = '\0';
	}
	friend void swap(CellGrid&, CellGrid&) noexcept;
	// friend int main(); // for debugging
};

CellGrid::iterator begin(CellGrid& grid) { return grid.begin(); }
CellGrid::iterator end(CellGrid& grid) { return grid.end(); }
//CellGrid::iterator rbegin(CellGrid& grid) { return grid.rbegin(); }
//CellGrid::iterator rend(CellGrid& grid) { return grid.rend(); }
CellGrid::iterator cbegin(CellGrid& grid) { return grid.cbegin(); }
CellGrid::iterator cend(CellGrid& grid) { return grid.cend(); }
//CellGrid::iterator crbegin(CellGrid& grid) { return grid.crbegin(); }
//CellGrid::iterator crend(CellGrid& grid) { return grid.crend(); }
void swap(CellGrid& grid, CellGrid& grid2) noexcept {
	auto w{grid.w}, h{grid.h}, esh{grid.esh}, gen{grid.gen};
	bool** dat{grid.dat}, ** act{grid.act},
	     * dst{grid.dst},  * dpv{grid.dpv}, * dcr{grid.dcr},
	     * acr{grid.acr},  * anx{grid.anx}, * als{grid.als},
	     * dlf{grid.dlf},  * drg{grid.drg},
	    started{grid.started}, vsh{grid.vsh};
	auto* pRow{grid.pRow};
	auto rule{grid.rule.code};
	char* rstr{grid.rule.srep},
	    tpid{grid.edge.id};
	grid.w = grid2.w;
	grid.h = grid2.h;
	grid.dat = grid2.dat;
	grid.act = grid2.act;
	grid.dst = grid2.dst;
	grid.dpv = grid2.dpv;
	grid.dcr = grid2.dcr;
	grid.acr = grid2.acr;
	grid.anx = grid2.anx;
	grid.als = grid2.als;
	grid.dlf = grid2.dlf;
	grid.drg = grid2.drg;
	grid.pRow = grid2.pRow;
	grid.started = grid2.started;
	grid.rule.code = grid2.rule.code;
	grid.rule.srep = grid2.rule.srep;
	grid.edge.id = grid2.edge.id;
	grid.esh = grid2.esh;
	grid.vsh = grid2.vsh;
	grid.gen = grid2.gen;
	grid2.w = w;
	grid2.h = h;
	grid2.dat = dat;
	grid2.act = act;
	grid2.dst = dst;
	grid2.dpv = dpv;
	grid2.dcr = dcr;
	grid2.acr = acr;
	grid2.anx = anx;
	grid2.als = als;
	grid2.dlf = dlf;
	grid2.drg = drg;
	grid2.pRow = pRow;
	grid2.started = started;
	grid2.rule.code = rule;
	grid2.rule.srep = rstr;
	std::swap(grid.rule.bArr, grid2.rule.bArr);
	std::swap(grid.rule.dArr, grid2.rule.dArr);
	grid2.edge.id = tpid;
	grid2.esh = esh;
	grid2.vsh = vsh;
	grid2.gen = gen;
}
void swap(CellGrid::Rule& rule, CellGrid::Rule& rule2) noexcept {
	auto code{rule.code};
	char* srep{rule.srep};
	rule.code = rule2.code;
	rule.srep = rule2.srep;
	rule2.code = code;
	rule2.srep = srep;
	std::swap(rule.bArr, rule2.bArr);
	std::swap(rule.dArr, rule2.dArr);
}
void swap(CellGrid::Topology& top, CellGrid::Topology& top2) noexcept {
	char id{top.id};
	top.id = top2.id;
	top2.id = id;
}
std::ostream& operator<<(std::ostream& out, const CellGrid& grid) {
	grid.print(false, out); // Switch this to printSafe(false, 0, 0, out) if the
	return out;             //  special characters used in print() do not work
}
std::istream& operator>>(std::istream& in, CellGrid::Rule& rule) {
	// Warning: This function currently always extracts 20 characters, even if the rulestring ends before then
	auto inw{ in.width() };
	in.width(21);
	char input[21];
	in >> input;
	if (!rule.readstr(input)) in.setstate(in.failbit);
	in.width(inw);
	return in;
}
std::ostream& operator<<(std::ostream& out, const CellGrid::Rule& rule) { return out << rule.cstr(); }
std::istream& operator>>(std::istream& in, CellGrid::Topology& top) {
	// this function might be the worst code i've ever written. enjoy :D
	char inCh;
	in >> inCh;
	constexpr signed char capDiff{ 'a' - 'A' }; // usually +32 but i want to make sure
	#define ct(ch) ((in >> inCh, inCh) == ch || inCh + capDiff == ch)
	switch (inCh) {
	case '0': case '1': case '2': case '3': case '4': case '5':
		top = CellGrid::Topology{ inCh };
		break;
	case 'P': case 'p':
		if (ct('l') && ct('a') && ct('n') && ct('e')) {
			top = CellGrid::plane;
			return in;
		}
		break;
	case 'T': case 't':
		if (ct('o') && ct('r') && ct('u') && ct('s')) {
			top = CellGrid::torus;
			return in;
		}
		break;
	case 'V': case 'v':
		in >> inCh;
		switch (inCh) {
		case 'E': case 'e':
			if (ct('r') && ct('t') && ct('i') && ct('c') && ct('a') && ct('l')) {
				in >> inCh;
				switch (inCh) {
				case ' ': case '\t': case '\v': case '\n': case '-': case '_':
					if (ct('b')) break;
				default:
					in.setstate(in.failbit);
					return in;
				}
			}
		case 'B': case 'b':
			break;
		default:
			in.setstate(in.failbit);
			return in;
		}
		[[fallthrough]];
	case 'B': case 'b':
		if (ct('o') && ct('t') && ct('t') && ct('l') && ct('e')) {
			top = CellGrid::vbottle;
			return in;
		}
		break;
	case 'H': case 'h':
		if (ct('o')) {
			if (ct('r') && ct('i') && ct('z') && ct('o') && ct('n') && ct('t') && ct('a') && ct('l'))
				switch(in >> inCh, inCh) {
				case ' ': case '\t': case '\v': case '\n': case '-': case '_':
					in >> inCh;
					break;
				default:
					in.setstate(in.failbit);
					return in;
				}
			else {
				in.setstate(in.failbit);
				return in;
			}
		}
		if (inCh == 'B' || inCh == 'b') {
			if (ct('o') && ct('t') && ct('t') && ct('l') && ct('e')) {
				top = CellGrid::hbottle;
				return in;
			}
		}
		break;
	case 'C': case 'c':
		if (ct('r') && ct('o') && ct('s') && ct('s')) switch(in >> inCh, inCh) {
		case ' ': case '\t': case '\v': case '\n': case '-': case '_':
			if (ct('s') && ct('u') && ct('r') && ct('f') && ct('a') && ct('c') && ct('e')) {
				top = CellGrid::cross;
				return in;
			}
		}
		break;
	case 'S': case 's':
		if (ct('p') && ct('h') && ct('e') && ct('r') && ct('e')) {
			top = CellGrid::sphere;
			return in;
		}
	}
	// figure out how to put stuff back in the stream
	in.setstate(in.failbit);
	return in;
}
std::ostream& operator<<(std::ostream& out, const CellGrid::Topology& top) { return out << top.cstr(); }

const CellGrid::Topology
	CellGrid::plane{'\0'}, // int 0 is confused with C-style null pointer
	CellGrid::torus{1},
	CellGrid::bottle{2},
	CellGrid::vbottle{2},
	CellGrid::hbottle{3},
	CellGrid::cross{4},
	CellGrid::sphere{5};
CellGrid::u32 CellGrid::defaultRule{ 0b00000110000000100i32 }; // == 3076 == "B3/S23" == Conway's Game of Life

#endif

/*
torus, +2 shift
----+-----+---
. o | . o | .
 +  |  +  |  +
+---+-+---+-+-
| . o | . o |
|  +  |  +  |
+-+---+-+---+-
o | . o | . o
  |  +  |  +
--+-----+-----
*/
