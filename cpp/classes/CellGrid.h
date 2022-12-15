#ifndef recentlymaterialized_CellGrid
#define recentlymaterialized_CellGrid
#include <iostream>
#include <cstdint>
#include <cassert>

// DISCLAIMER
// i am a novice at c++. this was written to help me learn, not to be useful to anyone. my past programming experience is JavaScript on an iPhone; because of this, everything from variable names to formatting is horrendously minimized. i truly apologize.

/***Usage***
> Instances of this class hold a grid of cells and can
  simulate basic cellular automata in a range of rules
> C++20-compliant compiler required
> Freely extensible: no private members

Constructors:
CellGrid(width, height, rule? = Life, generation? = 0)
CellGrid(initialGrid, width, height, rule?, generation?)
CellGrid(initialGrid, rule?, generation?)

CellGrid myGrid(1024, 512, "B3/S23"); // New empty 1024x512 Life grid
bool array2d[1024][96];
CellGrid slimGrid(array2d, 96, 1024); // New 96x1024 Life grid from array2d
CellGrid copyGrid(myGrid, '', 12); // Makes a copy of myGrid with generation=12
                                  (a two-dimensional container class with a
								  .size() member function may alternatively
								   be used as initialGrid here)

myGrid(30, 20) // Gets value of cell at x:30, y:20
myGrid(30, 20, 1) // Sets cell at x:30, y:20 to ON
myGrid( 8, 80, 0) // Sets cell at x:8, y:80 to OFF

myGrid++; // Advances grid one generation (may be prefix or postfix)
myGrid += 16; // Advances 16 generations

myGrid.width = 25;      // Width, height, generation, and
myGrid.height = 800;       rule may be changed freely as
myGrid.gen = 4001;         desired (note: B0 not allowed)
myGrid.rule = "B36/S23";
myGrid.rule = copyGrid.rule = 0b00000110000100100;

myGrid.count()      // Returns the number of ON  cells in grid
myGrid.count(false) // Returns the number of OFF cells in grid
myGrid.resize(width, height, keepCells = true) // resizes grid, clearing cells if keepCells is false
                                                (this can also be done by setting .width and .height)
myGrid.data() // Returns const bool* const* access to the internal grid (array of rows of bools) [deleted on resize]

myGrid.print()     // Prints grid to the console via std::ostream
myGrid.print(true) // Prints grid and generation number
  (The grid is printed using char values 220, 223, 219, and space to display two cells per character)
myGrid.printSafe(false, '#') // Prints grid without special characters (4x larger: two characters per cell)

myGrid = std::move(oldGrid); // move semantics are allowed, but the rvalue CellGrid
                                (oldGrid) is left in an unusable state
oldGrid.isValid() // returns false if contents have been moved away, otherwise true

CellGrid::Rule member functions:
myGrid.rule.b(n) // Returns true if an OFF cell becomes ON with n neighbors (birth)
myGrid.rule.s(n) // Returns true if an ON cell stays ON with n neighbors (survival)
myGrid.rule.d(n) // Returns true if an ON cell becomes OFF with n neighbors (death)
myGrid.rule.birthArr(),
myGrid.rule.deathArr() // Returns array of 9 const bools for each possible number of neighbors (0-8)
(unsigned long)(myGrid.rule) // Converts to a 17-bit integer of B/S behavior (Life = 3076)
(char*)(myGrid.rule) OR myGrid.rule.str() // Returns a human-readable string ("B3/S23")
                                           (allocated & created on first run; dies when rule changes or dies)
myGrid.rule = "..." // Sets rule to rulestring "...", returns true if successful, false if rulestring is invalid

const CellGrids and Rules are generally not supported
***********/

/*
  to do:
  - clean up the ridiculous mess i've made with the constructors
  - add swap functions
  - integrate some std::algorithms to make simulations faster (or learn parallelism)
  - add options for different edge behaviors (in progress!)
  - support some new families of rules (isotropic, generations, LtL)
  - use this to make something with graphics (help)
  - look up stuff about optimizing cellular automata

  HOW IS GOLLY SO FAST????
  i read through the quicklife code and understood nothing :'(
*/

// source:  https://github.com/recentlymaterialized/cellular-automata/blob/main/cpp/classes/CellGrid.h
// license: https://github.com/recentlymaterialized/cellular-automata/blob/main/LICENSE  (MIT license)

class CellGrid {
	using u32 = std::uint_fast32_t; // B/S rule codes (currently use 17 bits)
	using i32 = std::int_fast32_t; // grid dimensions are 32 bits
	static_assert(sizeof(i32) <= sizeof(size_t));
protected:
	static constexpr u32 defaultRule{ 0b00000110000000100 }; // B3/S23 Conway's Game of Life
	bool** dat, ** act, // stores grid and active cells
		* dst{nullptr}, * dpv{nullptr}, * dcr{nullptr}, // helper arrays for calculations
		* acr{nullptr}, * anx{nullptr}, * als{nullptr};
	bool started{}; // if false, act is treated as uninitialized
	char* pRow{nullptr}; // helper array for printing
	i32 w, h;
	i32 planeCount(i32 x, i32 y) {
		if (y) {if (y == h - 1) { // why am i optimizing an edge case
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
	i32 shTorusCount(i32 x, i32 y) {}
	i32 crossesCount(i32 x, i32 y) {} // horz. bottle, vert. bottle, and cross-surface
	i32 shCrossesCount(i32 x, i32 y) {}
	i32 sphereCount(i32 x, i32 y) {}
	// i don't think shift works with spheres? i will look into it
	void planeSet(i32 x, i32 y, bool state) {
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
	i32 shTorusSet(i32 x, i32 y) {}
	i32 shTorusSetFl(i32 x, i32 y) {}
	i32 crossesSet(i32 x, i32 y) {}
	i32 crossesSetFl(i32 x, i32 y) {}
	i32 shCrossesSet(i32 x, i32 y) {}
	i32 shCrossesSetFl(i32 x, i32 y) {}
	i32 sphereSet(i32 x, i32 y) {}
	i32 sphereSetFl(i32 x, i32 y) {}
	void weakResize() noexcept {
		assert(width > 0 && height > 0); // asserts don't count as exceptions right...?
		w = width;
		h = height;
		delete[] dst;
		delete[] dpv;
		delete[] dcr;
		delete[] acr;
		delete[] anx;
		delete[] als;
		delete[] pRow;
		dst = nullptr;
		dpv = nullptr;
		dcr = nullptr;
		acr = nullptr;
		anx = nullptr;
		als = nullptr;
		pRow = nullptr;
	}
public:
	enum EdgeBhvr { plane, torus, vbottle, hbottle, cross, sphere, bottle = 2 };
protected:
	void primeEdge() {
		for (i32 i{}; i != w; ++i) act[0][i] = act[h-1][i] = true;
		for (i32 i{}; i != h; ++i) act[i][0] = act[i][w-1] = true;
	}
	class Rule {
		friend CellGrid;
		CellGrid* const grid;
		u32 code;
		char* srep{nullptr};
		bool bArr[9], dArr[9];
		EdgeBhvr ebh;
		i32 esh{};
		bool vsh{};
	public:
		Rule(CellGrid* gd) noexcept : grid{ gd } {} // Use sparingly
		Rule(CellGrid* gd, u32 cd, EdgeBhvr egbh = plane) noexcept
		: grid{ gd }, code{ cd }, ebh{ egbh } {
			if (!code) code = defaultRule; // initialization with 0 defaults to Life; this is not the case for assignment
			if (code == -1) code = 131072; // rule 0 "B/S" stored as 2^17
			u32 pos{1};
			bArr[0] = false;
			for (int i{1}; i != 9; ++i, pos <<= 1) bArr[i] = code & pos;
			for (int i{0}; i != 9; ++i, pos <<= 1) dArr[i] = ~code & pos;
		}
		Rule(CellGrid* gd, const Rule& rule, EdgeBhvr egbh = plane)
		: grid{ gd }, code{ rule.code }, ebh{ egbh } {
			for (int i{}; i != 9; ++i) bArr[i] = rule.bArr[i];
			for (int i{}; i != 9; ++i) dArr[i] = rule.dArr[i];
		}
		Rule(CellGrid* gd, Rule&& rule, EdgeBhvr egbh = plane) noexcept
		: grid{ gd }, code{ rule.code }, ebh{ egbh } {
			for (int i{}; i != 9; ++i) bArr[i] = rule.bArr[i]; // these must be copied because they
			for (int i{}; i != 9; ++i) dArr[i] = rule.dArr[i]; //  are not dynamically allocated :(
			if (rule.srep) {
				srep = rule.srep;
				rule.srep = nullptr;
			}
		}
		Rule(CellGrid* gd, char* rulestr, EdgeBhvr egbh = plane)
		: grid{ gd }, ebh{ egbh } {
			if (!(operator=(rulestr))) throw "Invalid rulestring";
		}
		virtual ~Rule() { delete[] srep; }
		u32 operator=(u32 cd) noexcept {
			if (!cd) cd = 131072;
			if (code != cd) {
				code = cd;
				u32 pos{1};
				for (int i{1}; i != 9; ++i, pos <<= 1) bArr[i] = code & pos;
				for (int i{0}; i != 9; ++i, pos <<= 1) dArr[i] = ~code & pos;
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
				for (i32 i{1}; i != 9; ++i) bArr[i] = rule.bArr[i];
				for (i32 i{0}; i != 9; ++i) dArr[i] = rule.dArr[i];
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
		bool operator=(char* string) {
			if (string == nullptr) {
				operator=(defaultRule); // Life
				return true;
			}
			if (*string != 'b' && *string != 'B') return false;
			u32 newCode{}, pos;
			signed char stage{}, num;
			for (int i{1}; i != 256; ++i) switch (string[i]) {
			case '\0':
				if (stage == 2) {
					if (!newCode) newCode = 131072;
					code = newCode;
					pos = 1;
					for (int i{1}; i != 9; ++i, pos <<= 1) bArr[i] = code & pos;
					for (int i{0}; i != 9; ++i, pos <<= 1) dArr[i] = ~code & pos;
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
				for (char i{}; i != num; ++i) pos <<= 1;
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
		bool operator==(u32 cd) { return code == cd; }
		bool operator==(Rule& rl) { return code == rl.code; }
		bool operator==(char* string) { return code == Rule{ nullptr, string }.code; }
		void edge(EdgeBhvr egbh) {
			if (grid->width != grid->w || grid->height != grid->h) grid->resize();
			else if (grid->started) grid->primeEdge();
			if (egbh > 4) throw "Invalid EdgeBhvr";
			ebh = egbh;
		}
		void shift(i32 shft) {
			if (grid->width != grid->w || grid->height != grid->h) grid->resize();
			else if (grid->started) grid->primeEdge();
			if (vsh) esh = shft % grid->h; // C++'s modulo % operator treats the dividend as an absolute
			else esh = shft % grid->w;     //   value and then multiplies by the sign of the dividend
		}
		void shift(i32 shft, bool vshft) {
			if (grid->width != grid->w || grid->height != grid->h) grid->resize();
			if (grid->started) grid->primeEdge();
			if (vshft) {
				esh = shft % grid->h;
				vshft = true;
			} else {
				esh = shft % grid->w;
				vshft = false;
			}
		}
		bool b(i32 i) const { return bArr[i]; }
		bool s(i32 i) const { return !dArr[i]; }
		bool d(i32 i) const { return dArr[i]; }
		const bool* birthArr() const { return bArr; }
		const bool* deathArr() const { return dArr; }
		bool canFlicker() const {
			for (int i{}; i != 9; ++i) if (bArr[i] && dArr[i]) return true;
			return false;
		}
		char* str() {
			if (!srep) {
				int i{}, count{4};
				u32 pos{1};
				for (; pos != 131072l; pos <<= 1) count += code & pos;
				srep = new char[count] {'B'};
				count = 0;
				for (pos = 1; pos != 256; ++i, pos <<= 1)
					if (code & pos) srep[++count] = i + '1';
				srep[++count] = '/';
				srep[++count] = 'S';
				for (i = 0; pos != 131072l; ++i, pos <<= 1)
					if (code & pos) srep[++count] = i + '0';
				srep[++count] = '\0';
			}
			return srep;
		}
		operator char* () { return str(); }
		operator u32() const { return code; }
	};
	void construct(i32 wdt, i32 hgt) {
		assert(wdt > 0 && wdt < 65536l && hgt > 0 && hgt < 65536l);
		dat = new bool*[hgt];
		act = new bool*[hgt];
		for (i32 i{}, j; i != hgt; ++i) {
			dat[i] = new bool[wdt];
			act[i] = new bool[wdt];
			for (j = 0; j != wdt; ++j) dat[i][j] = false;
		}
	}
	void moveConstruct(CellGrid&& grid) {
		dat = grid.dat;
		act = grid.act;
		dst = grid.dst;
		dpv = grid.dpv;
		dcr = grid.dcr;
		acr = grid.acr;
		anx = grid.anx;
		als = grid.als;
		pRow = grid.pRow;
		grid.dat = grid.act = nullptr;
		grid.dst = grid.dpv = grid.dcr =
		grid.acr = grid.anx = grid.als = nullptr;
		grid.pRow = nullptr;
		started = grid.started;
	}
public:
	Rule rule;
	i32 gen;
	i32 width, height;
	CellGrid(i32 wdt, i32 hgt, u32 rl, EdgeBhvr egbh = plane, i32 gn = 0)
	  : w{wdt}, width{wdt}, h{hgt}, height{hgt}, gen{gn}, rule{this, rl, egbh}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, char* rl, EdgeBhvr egbh = plane, i32 gn = 0)
	  : w{wdt}, width{wdt}, h{hgt}, height{hgt}, gen{gn}, rule{this, rl, egbh}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, const Rule& rl, EdgeBhvr egbh = plane, i32 gn = 0)
	  : w{wdt}, width{wdt}, h{hgt}, height{hgt}, gen{gn}, rule{this, rl, egbh}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, Rule&& rl, EdgeBhvr egbh = plane, i32 gn = 0)
	  : w{wdt}, width{wdt}, h{hgt}, height{hgt}, gen{gn}, rule{this, std::move(rl), egbh}
		{ construct(wdt, hgt); }
	CellGrid(i32 wdt, i32 hgt, EdgeBhvr egbh = plane) : w{wdt}, width{wdt}, h{hgt}, height{hgt}, gen{}, rule{this, 0u, egbh}
		{ construct(wdt, hgt); }          // default edge behavior is now plane, not torus!
	CellGrid(const CellGrid& grid, char* rl, EdgeBhvr egbh, i32 gn) : CellGrid(grid.width, grid.height, rl, egbh, gn) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, char* rl, EdgeBhvr egbh) : CellGrid(grid.width, grid.height, rl, egbh, grid.gen) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, char* rl) : CellGrid(grid.width, grid.height, rl, grid.rule.ebh, grid.gen) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, const Rule& rl, EdgeBhvr egbh, i32 gn) : CellGrid(grid.width, grid.height, rl, egbh, gn) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, const Rule& rl, EdgeBhvr egbh) : CellGrid(grid.width, grid.height, rl, egbh, grid.gen) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, const Rule& rl) : CellGrid(grid.width, grid.height, rl, grid.rule.ebh, grid.gen) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, Rule&& rl, EdgeBhvr egbh, i32 gn) : CellGrid(grid.width, grid.height, std::move(rl), egbh, gn) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, Rule&& rl, EdgeBhvr egbh) : CellGrid(grid.width, grid.height, std::move(rl), egbh, grid.gen) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, Rule&& rl) : CellGrid(grid.width, grid.height, std::move(rl), grid.rule.ebh, grid.gen) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, u32 rl, EdgeBhvr egbh, i32 gn) : CellGrid(grid.width, grid.height, rl, egbh, gn) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	// the constructor (CellGrid&, u32, EdgeBhvr) would be confused by the compiler with (CellGrid&, i32, i32) :(
	CellGrid(const CellGrid& grid, u32 rl) : CellGrid(grid.width, grid.height, rl, grid.rule.ebh, grid.gen) {
		for (i32 i{}, j; i != grid.h && i != grid.height; ++i)
			for (j = 0; j != grid.w && j != grid.width; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid) : CellGrid(grid, grid.rule, grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, char* rl, EdgeBhvr egbh, i32 gn) : rule{ this } {
		moveConstruct(std::move(grid));
		if (!(rule = rl)) throw "Invalid rule";
		rule.ebh = egbh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, char* rl, EdgeBhvr egbh) : CellGrid(std::move(grid), rl, egbh, grid.gen) {}
	CellGrid(CellGrid&& grid, char* rl) : CellGrid(std::move(grid), rl, grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, const Rule& rl, EdgeBhvr egbh, i32 gn) : rule{ this } {
		moveConstruct(std::move(grid));
		rule = rl;
		rule.ebh = egbh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, const Rule& rl, EdgeBhvr egbh) : CellGrid(std::move(grid), rl, egbh, grid.gen) {}
	CellGrid(CellGrid&& grid, const Rule& rl) : CellGrid(std::move(grid), rl, grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, Rule&& rl, EdgeBhvr egbh, i32 gn) noexcept : rule{ this } {
		moveConstruct(std::move(grid));
		rule = std::move(rl);
		rule.ebh = egbh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, Rule&& rl, EdgeBhvr egbh) : CellGrid(std::move(grid), std::move(rl), egbh, grid.gen) {}
	CellGrid(CellGrid&& grid, Rule&& rl) : CellGrid(std::move(grid), std::move(rl), grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, u32 rl) : rule{ this } {
		moveConstruct(std::move(grid));
		rule = rl;
		rule.ebh = grid.rule.ebh;
		gen = grid.gen;
	}
	CellGrid(CellGrid&& grid) noexcept : CellGrid(std::move(grid), std::move(grid.rule), grid.rule.ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, char* rl, EdgeBhvr egbh, i32 gn) : CellGrid(wdt, hgt, rl, egbh, gn) {
		for (i32 i{}, j; i != grid.h && i != hgt; ++i)
			for (j = 0; j != grid.w && j != wdt; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, char* rl, EdgeBhvr egbh) : CellGrid(grid, wdt, hgt, rl, egbh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, char* rl) : CellGrid(grid, wdt, hgt, rl, grid.rule.ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const Rule& rl, EdgeBhvr egbh, i32 gn) : CellGrid(wdt, hgt, rl, egbh, gn) {
		for (i32 i{}, j; i != grid.h && i != hgt; ++i)
			for (j = 0; j != grid.w && j != wdt; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const Rule& rl, EdgeBhvr egbh) : CellGrid(grid, wdt, hgt, rl, egbh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, const Rule& rl) : CellGrid(grid, wdt, hgt, rl, grid.rule.ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, Rule&& rl, EdgeBhvr egbh, i32 gn) : CellGrid(wdt, hgt, std::move(rl), egbh, gn) {
		for (i32 i{}, j; i != grid.h && i != hgt; ++i)
			for (j = 0; j != grid.w && j != wdt; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, Rule&& rl, EdgeBhvr egbh) : CellGrid(grid, wdt, hgt, std::move(rl), egbh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, Rule&& rl) : CellGrid(grid, wdt, hgt, std::move(rl), grid.rule.ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, u32 rl, EdgeBhvr egbh, i32 gn) : CellGrid(wdt, hgt, rl, egbh, gn) {
		for (i32 i{}, j; i != grid.h && i != hgt; ++i)
			for (j = 0; j != grid.w && j != wdt; ++j) dat[i][j] = grid.dat[i][j];
	}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, u32 rl, EdgeBhvr egbh) : CellGrid(grid, wdt, hgt, rl, egbh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt, u32 rl) : CellGrid(grid, wdt, hgt, rl, grid.rule.ebh, grid.gen) {}
	CellGrid(const CellGrid& grid, i32 wdt, i32 hgt) : CellGrid(grid, wdt, hgt, grid.rule, grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, char* rl, EdgeBhvr egbh, i32 gn) : rule{ this } {
		moveConstruct(std::move(grid));
		width = wdt;
		height = hgt;
		if (started) {
			started = false;
			if (!(rule = rl)) throw "Invalid rule";
			started = true;
		} else if (!(rule = rl)) throw "Invalid rule";
		rule.ebh = egbh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, char* rl, EdgeBhvr egbh) : CellGrid(std::move(grid), wdt, hgt, rl, egbh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, char* rl) : CellGrid(std::move(grid), wdt, hgt, rl, grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const Rule& rl, EdgeBhvr egbh, i32 gn) : rule{ this } {
		moveConstruct(std::move(grid));
		width = wdt;
		height = hgt;
		if (started) {
			started = false;
			rule = rl;
			started = true;
		} else rule = rl;
		rule.ebh = egbh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const Rule& rl, EdgeBhvr egbh) : CellGrid(std::move(grid), wdt, hgt, rl, egbh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, const Rule& rl) : CellGrid(std::move(grid), wdt, hgt, rl, grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, Rule&& rl, EdgeBhvr egbh, i32 gn) : rule{ this } {
		moveConstruct(std::move(grid));
		width = wdt;
		height = hgt;
		if (started) {
			started = false;
			rule = std::move(rl);
			started = true;
		} else rule = std::move(rl);
		rule.ebh = egbh;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, Rule&& rl, EdgeBhvr egbh) : CellGrid(std::move(grid), wdt, hgt, std::move(rl), egbh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, Rule&& rl) : CellGrid(std::move(grid), wdt, hgt, std::move(rl), grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, u32 rl, EdgeBhvr egbh, i32 gn) : rule{ this } {
		moveConstruct(std::move(grid));
		width = wdt;
		height = hgt;
		if (started) {
			started = false;
			rule = rl;
			started = true;
		} else rule = rl;
		gen = gn;
	}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, u32 rl, EdgeBhvr egbh) : CellGrid(std::move(grid), wdt, hgt, rl, egbh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt, u32 rl) : CellGrid(std::move(grid), wdt, hgt, rl, grid.rule.ebh, grid.gen) {}
	CellGrid(CellGrid&& grid, i32 wdt, i32 hgt) : CellGrid(std::move(grid), wdt, hgt, std::move(grid.rule), grid.rule.ebh, grid.gen) {}
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, char* rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, Rule& rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, Rule&& rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, std::move(rl), egbh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt, u32 rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(const bool* const* cells, i32 wdt, i32 hgt) : CellGrid(wdt, hgt)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, char* rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ dat = cells; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, Rule& rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ dat = cells; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, Rule&& rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, std::move(rl), egbh, gn)
		{ dat = cells; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt, u32 rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ dat = cells; }
	CellGrid(bool**&& cells, i32 wdt, i32 hgt) : CellGrid(wdt, hgt)
		{ dat = cells; }
	template<class T> CellGrid(T cells, i32 wdt, i32 hgt, char* rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<class T> CellGrid(T cells, i32 wdt, i32 hgt, Rule& rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<class T> CellGrid(T cells, i32 wdt, i32 hgt, Rule&& rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, std::move(rl), egbh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<class T> CellGrid(T cells, i32 wdt, i32 hgt, u32 rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid(wdt, hgt, rl, egbh, gn)
		{ for (i32 i{}, j; i != hgt; ++i) for (j = 0; j != wdt; ++j) dat[i][j] = cells[i][j]; }
	template<class T> CellGrid(T cells, char* rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid((*cells).size(), cells.size(), rl, egbh, gn)
		{ for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) dat[i][j] = cells[i][j]; }
	template<class T> CellGrid(T cells, Rule& rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid((*cells).size(), cells.size(), rl, egbh, gn)
		{ for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) dat[i][j] = cells[i][j]; }
	template<class T> CellGrid(T cells, Rule&& rl, EdgeBhvr egbh = plane, i32 gn = 0) : CellGrid((*cells).size(), cells.size(), std::move(rl), egbh, gn)
		{ for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) dat[i][j] = cells[i][j]; }
	template<class T> CellGrid(T cells, i32 rl, EdgeBhvr egbh, i32 gn = 0) : CellGrid((*cells).size(), cells.size(), rl, egbh, gn)
		{ for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) dat[i][j] = cells[i][j]; }
		// egbh cannot be optional: if egbh is optional, the compiler could confuse this with the (i32 width, i32 height) constructor.
	template<class T> CellGrid(T cells) : CellGrid((*cells).size(), cells.size())
		{ for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) dat[i][j] = cells[i][j]; }
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
		delete[] pRow;
	}
	CellGrid& operator=(CellGrid& grid) {
		if (&grid == this) return *this;
		if (grid.width != grid.w || grid.height != grid.h) grid.resize();
		if (w != grid.w || h != grid.h) resize(grid.w, grid.h, false);
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
		pRow = grid.pRow;
		grid.dat = grid.act = nullptr;
		grid.dst = grid.dpv = grid.dcr =
		grid.acr = grid.anx = grid.als = nullptr;
		started = false;
		rule = std::move(grid.rule);
		started = grid.started;
		gen = grid.gen;
		w = grid.w;
		h = grid.h;
		width = grid.width;
		height = grid.height;
		return *this;
	}
	CellGrid& operator=(const bool* const* grid) { // make sure input grid has the same dimensions as CellGrid!!
		if (w == width) { if (h != h)
			assert(height > 0),
			h = height; }
		else weakResize();
		for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j)
			dat[i][j] = grid[i][j],
			act[i][j] = true;
		started = false;
		return *this;
	}
	CellGrid& operator=(bool**&& grid) noexcept { // especially here!!! the error will not surface immediately
		if (w == width) { if (h != h)
			assert(height > 0),
			h = height; }
		else weakResize();
		if (grid == dat) return *this;
		delete[] dat;
		dat = grid;
		for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) act[i][j] = true;
		started = false;
		return *this;
	}
	const bool* const* data() {
		if (width != w || height != h) resize();
		return dat;
	}
	const bool* const* active() { return act; } // for debugging | warning: act may be uninitialized
	bool operator()(i32 x, i32 y) const {
		if (x < 0 || x >= w || y < 0 || w >= h) return false;
		return dat[y][x];
	}
	bool operator()(i32 x, i32 y, bool value) {
		assert(x >= 0 && x < width && y >= 0 && y < height);
		if (width != w && x >= w || height != h && y >= h) resize();
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
	CellGrid& operator++() {
		if (width != w || height != h) resize();
		if (!dst) {
			dst = new bool[width];
			dpv = new bool[width];
			dcr = new bool[width];
			acr = new bool[width];
			anx = new bool[width];
			als = new bool[width];
		}
		bool flicker{ rule.canFlicker() };
		const bool* birth{ rule.birthArr() };
		const bool* death{ rule.deathArr() };
		const i32 wm1{ w - 1 }, hm1{ h - 1 };
		auto edgeCount{ &CellGrid::torusCount }; // C++ is confusing
		auto edgeSet{ flicker ? &CellGrid::torusSetFl : &CellGrid::torusSet };
		if (!started) {
			for (i32 i{}, j; i != h; ++i) {
				bool* asp{ i ? act[i - 1] : act[hm1] },
				    * asc{ act[i] },
				    * asn{ i == hm1 ? act[i + 1] : *act };
				for (j = 0;;) {
					if (i && i != hm1 && j && j != -1) {
						if (flicker) {
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
						}
						else {
							if (dat[i][j])
								asp[j-1] = asp[j] = asp[j+1] =
								asc[j-1]          = asc[j+1] =
								asn[j-1] = asn[j] = asn[j+1] = true;
							++j;
							if (j == wm1) j = -1;
						}
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
		int neighbors;
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
		++gen;
		return *this;
	}
	CellGrid& operator++(int) { return operator++(); }
	CellGrid& operator+=(int gens) {
		for (int i{}; i != gens; ++i) operator++();
		return *this;
	}
	CellGrid operator+(int gens) {
		CellGrid newGrid{*this};
		for (int i{}; i != gens; ++i) ++newGrid;
		return newGrid;
	}
	long count(bool state = true) const {
		long result{};
		if (width < w || height < h) for (i32 i{}, j; i != height; ++i) for (j = 0; j != width; ++j)
			{ if (dat[i][j]) result++; }
		else for (i32 i{}, j; i != h; ++i) for (j = 0; j != w; ++j) if (dat[i][j]) result++;
		if (state) return result;
		else return (long)width * (long)height - result;
	}
	void resize(i32 wdt = 0, i32 hgt = 0, bool keepCells = true, bool center = false) {
		if (wdt) width = wdt;
		if (hgt) height = hgt;
		if (w == width) {
			if (h == height) {
				if (!keepCells) clear();
				return;
			}
			assert(height > 0);
		}
		else {
			assert(width > 0 && height > 0);
			delete[] dst;
			delete[] dpv;
			delete[] dcr;
			delete[] acr;
			delete[] anx;
			delete[] als;
			dst = nullptr;
			dpv = nullptr;
			dcr = nullptr;
			acr = nullptr;
			anx = nullptr;
			als = nullptr;
		}
		#define clhf(num) ( num < 0 ? num / 2 : (num + 1) / 2 )
		if (h == height) {
			bool* rda, * rac;
			for (i32 i{}, j; i != h; ++i) {
				if (keepCells) {
					rda = new bool[width];
					rac = new bool[width];
					j = 0;
					if (center) for (i32 hoff{ clhf((i32)w - (i32)width) }; j != w; ++j) rda[j] =
						j + hoff >= 0 && j + hoff < w ? dat[i][j + hoff] : false;
					else for (; j != w; ++j) rda[j] = dat[i][j];
				} else rda = new bool[width]{},
				       rac = new bool[width]{};
				delete[] dat[i];
				delete[] act[i];
				dat[i] = rda;
				act[i] = rac;
			}
			w = width;
		}
		else {
			bool** nda{ new bool*[height] },
			    ** nac{ new bool*[height] };
			for (i32 i{}, j; i != height; ++i) {
				nda[i] = new bool[width];
				nac[i] = new bool[width];
				j = 0;
				if (keepCells) {
					if (center) {
						for (i32 hoff{ clhf((i32)w - (i32)width) },
						         ysrc{ (i32)i + clhf((i32)h - (i32)height) }; j != width; ++j)
							if (ysrc >= 0 && ysrc < h && j + hoff >= 0 && j + hoff < w)
								nda[i][j] = dat[ysrc][j + hoff],
								nac[i][j] = act[ysrc][j + hoff];
							else nda[i][j] = nac[i][j] = false;
					}
					else for (; j != width; ++j)
						nda[i][j] = dat[i][j],
						nac[i][j] = act[i][j];
				}
				else for (; j != width; ++j) nda[i][j] = false, nac[i][j] = false;
				if (i < h)
					delete[] dat[i],
					delete[] act[i];
			}
			if (keepCells) {
				i32 i{};
				if (center) {
					i32 hoff{ ((i32)width - (i32)w) / 2 }, voff{ ((i32)height - (i32)h) / 2 };
					if (w < width) i = hoff;
					if (h < height) {
						for (; i - hoff != w && i != width; ++i) if (i >= 0)
							(voff ? nac[voff-1][i] : nac[height-1][i]) = nac[voff][i]
							  = nac[h-1 + voff][i] = nac[h + voff][i] = true;
						i = voff;
					}
					else {
						for (; i - hoff != w && i != width; ++i) if (i >= 0)
							nac[0][i] = nac[height - 1][i] = true;
						i = 0;
					}
					if (w < width) for (; i - voff != h && i != height; ++i) if (i >= 0)
						(hoff ? nac[i][hoff-1] : nac[i][width-1]) = nac[i][hoff]
						  = nac[i][w-1 + hoff] = nac[i][w + hoff] = true;
					else for (; i - voff != h && i != height; ++i) if (i >= 0)
						nac[i][0] = nac[i][width-1] = true;
					// fix these                vvv
					nac[height-1][width-1] = true;
					if (w < width) nac[height-1][w] = (h > height) ? true : nac[h][w] = true;
					if (h < height) nac[h][width-1] = (w >  width) ? true : nac[h][w] = true;
				}
				else {
					if (h < height) for (; i != w; ++i)
						nac[0][i] = nac[h-1][i] = nac[h][i] = nac[height-1][i] = true;
					else for (; i !=  width; ++i) nac[0][i] = nac[height-1][i] = true;
					if (w < width) for (i = 0; i != h; ++i)
						nac[i][0] = nac[i][w-1] = nac[i][w] = nac[i][width-1] = true;
					else for (i = 0; i != height; ++i) nac[i][0] = nac[i][width-1] = true;
					nac[height-1][width-1] = true;
					if (w < width) nac[height-1][w] = (h >= height) ? true : nac[h][w] = true;
					if (h < height) nac[h][width-1] = (w >=  width) ? true : nac[h][w] = true;
				}
			}
			delete[] dat;
			delete[] act;
			dat = nda;
			act = nac;
			w = width;
			h = height;
		}
	}
	void resize(bool keepCells, bool center = false) { resize(0, 0, keepCells, center); }
	void clear() {
		for (i32 i{}, j, wdt{ width < w ? width : w }, hgt{ height < h ? height : h }; i != hgt; ++i)
			for (j = 0; j != wdt; ++j) dat[i][j] = act[i][j] = false;
	}
	bool isValid() const { return dat; }
	void print(bool pGen = false) {
		if (width != w || height != h) resize();
		if (!pRow) pRow = new char[w + 2];
		if (pGen) std::cout << "Generation " << gen << '\n';
		for (i32 i{}, j; i < h; i += 2) {
			j = 0;
			if (i + 1 == h) for (; j != w; ++j) pRow[j] = dat[i][j] ? 223 : ' ';
			else for (; j != w; ++j) pRow[j] =
				dat[i][j] ? (dat[i+1][j] ? 219 : 223) : (dat[i+1][j] ? 220 : ' ');
			pRow[w] = '\n';
			pRow[w+1] = '\0';
			std::cout << pRow;
		}
		*pRow = static_cast<unsigned char>(255);
	}
	void printSafe(bool pGen = false, char ch1 = '\0', char ch2 = '\0') {
		if (width != w || height != h) resize();
		if (ch1) { if (!ch2) ch2 = ch1; }
		else ch1 = '[', ch2 = ']';
		if (!pRow) pRow = new char[w * 2 + 2];
		else if (*pRow == 255) {
			delete[] pRow;
			pRow = new char[w * 2 + 2];
		}
		if (pGen) std::cout << "Generation " << gen << '\n';
		for (i32 i{}, j; i < h; ++i) {
			for (j = 0; j != w; ++j) {
				pRow[j*2] = dat[i][j] ? ch1 : ' ';
				pRow[j*2+1] = dat[i][j] ? ch2 : ' ';
			}
			pRow[w*2] = '\n';
			pRow[w*2+1] = '\0';
			std::cout << pRow;
		}
	}
	friend std::ostream& operator<<(std::ostream& out, CellGrid grid) {
		grid.print();
		return out;
	}
	friend std::ostream& operator<<(std::ostream& out, Rule rule) {
		out << rule.str();
		return out;
	}
};
#endif
