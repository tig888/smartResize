#pragma once

#include<vector>

std::vector<unsigned char> smartResize( const std::vector<unsigned char>* sourceImage,
                                        int sourceImageWidth,
                                        int desiredWidth,
                                        int desiredHeight,
                                        const std::vector<unsigned char>* mask = nullptr);