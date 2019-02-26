#pragma once

#include<iostream>
#include<vector>
#include"Image.hpp"

class SmartImageScale{
	const ImageRGBA<unsigned char> origImage, origMask;
	ImageRGBA<unsigned char> workImage, workMask;

	void verticalAdjust(int N);
	void horizontalAdjust(int N);

	public:
	SmartImageScale(const std::vector<unsigned char>& input, int width);
	SmartImageScale(const std::vector<unsigned char>& input, const std::vector<unsigned char>& mask, int width);

	ImageRGBA<unsigned char> scale(int desiredWidth, int desiredHeight);
};