#pragma once

#include<iostream>
#include<vector>
#include"Image.hpp"

class SmartImageScale{
	const ImageRGBA<unsigned char> origImage, origMask;
	ImageRGBA<unsigned char> workImage, workMask;

	auto verticalAdjust(int N)		-> void;
	auto horizontalAdjust(int N)	-> void;

	public:
	SmartImageScale(const std::vector<unsigned char>& input, int width);
	SmartImageScale(const std::vector<unsigned char>& input, const std::vector<unsigned char>& mask, int width);

	auto scale(int desiredWidth, int desiredHeight) -> ImageRGBA<unsigned char>;
};