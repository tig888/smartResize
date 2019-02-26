#include"smartResize.hpp"
#include"SmartImageScale.hpp"
#include<vector>

using namespace std;

vector<unsigned char> smartResize(  const vector<unsigned char>* sourceImage,
                                    int sourceImageWidth,
                                    int desiredWidth,
                                    int desiredHeight,
                                    const vector<unsigned char>* mask){

    vector<unsigned char> empty;
    if(desiredHeight < 4 || desiredWidth < 4) return empty;
    if(mask == nullptr) {
        SmartImageScale sis(*sourceImage, sourceImageWidth);
        return sis.scale(desiredWidth, desiredHeight);
    }
    else{
        if(mask->size() != sourceImage->size()) return empty;
        SmartImageScale sis(*sourceImage, *mask, sourceImageWidth);
        return sis.scale(desiredWidth, desiredHeight);
    }
}