#pragma once
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

/*
 * Misc utilities that don't use natives and would clutter the main
 * classes.
 */

namespace NativeMenu {
	struct Color {
		int R;
		int G;
		int B;
		int A;
	};

	struct Sprite {
		std::string Dictionary;
		std::string Name;
	};

	const Color solidWhite = { 255,	255, 255, 255 };
	const Color solidBlack = { 0, 0, 0, 150 };

	const Color solidRed = { 255, 0, 0,	255 };
	const Color solidGreen = { 0, 255, 0, 255 };
	const Color solidBlue = { 0, 0, 255, 255 };

	const Color solidPink = { 255, 0, 255, 255 };
	const Color solidYellow = { 255, 255, 0, 255 };
	const Color solidCyan = { 0, 255, 255, 255 };

	const Color solidOrange = { 255, 127, 0, 255 };
	const Color solidLime = { 127, 255, 0, 255 };
	const Color solidPurple = { 127, 0, 255, 255 };

	const Color transparentGray = { 75, 75, 75, 75 };

	unsigned numZeroes(unsigned number);
	unsigned behindDec(float f);

	// https://stackoverflow.com/questions/236129/split-a-string-in-c
	template<typename Out>
	void split(const std::string &s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}

	std::vector<std::string> split(const std::string &s, char delim);

	//std::string makeCaps(std::string input);
}
