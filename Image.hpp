#pragma once

#include<vector>
#include<iostream>
#include<lodepng.h>
#include<array>
#include<string>
#include<cmath>
#include<set>

typedef float FP;


template<typename T, int STEP = 1>
class Data2D : public std::vector<T>{
    protected:

    using std::vector<T>::push_back;
    using std::vector<T>::emplace_back;
    using std::vector<T>::pop_back;
    public:
    int width, height;
    Data2D() : width(0), height(0){
    }
    Data2D( int width_, int height_) : width(width_), height(height_), std::vector<T>(width_*height_*STEP){
    }
    Data2D( const std::vector<T>& data_, int width_) : std::vector<T>(data_), width(width_){
        if(data_.size() > 0) height = data_.size()/(width_*STEP);
        else height = 0;
    }
    Data2D( std::vector<T>&& data_, int width_) : std::vector<T>(data_), width(width_){
        if(data_.size() > 0) height = data_.size()/(width_*STEP);
        else height = 0;
    }
    Data2D(const Data2D<T, STEP>& other)            = default;
    Data2D& operator=(const Data2D<T, STEP>& data_) = default;
    Data2D& operator=( Data2D<T,STEP>&& other)      = default;
    Data2D(Data2D&& other)                          = default;

    inline int  pixels()    const { return width * height ;}
    inline int  rows()      const { return height;}
    inline int  getHeight() const { return height;}
    inline int  cols()      const { return width ;}
    inline int  getWidth()  const { return width ;}
    inline int  idx(T* ptr) { return (ptr - this->data())/STEP ;}

	inline T*  below(T* ptr) { return ptr + width * STEP;}
	inline T*  above(T* ptr) { return ptr - width * STEP;}
	inline T*  right(T* ptr) { return ptr + STEP;}
	inline T*  left(T* ptr) { return ptr - STEP;}

    inline const T*  getPtr(int idx) const { return this->data() + idx*STEP;}
    inline const T*  getPtr(int x, int y) const{ return this->data() + (y * width + x) * STEP;}

    inline T*  accessPtr(int idx)  { return this->data() + idx*STEP;}
    inline T*  accessPtr(int x, int y)  { return this->data() + (y * width + x) * STEP;}

    inline T*  endPtr()  { return this->data() + ( width * height * STEP);}
    inline T*  lastPtr()  { return endPtr - STEP;}


    inline T  get(int x, int y) const { return *(accessPtr(x,y));}

	inline bool leftEdge(T* ptr) const { return idx(ptr) % width == 0;}
	inline bool rightEdge(T* ptr) const { return (idx(ptr) + 1) % width == 0;}
	inline bool bottomEdge(T* ptr) const { return idx(ptr) >= width * (height - 1) ;}

    Data2D<T,STEP> rotate90CCW(){
        Data2D<T,STEP> output(height, width);
        for(int x = 0; x < width ; x++){
            for(int y = 0; y < height ; y++){
                T* dest = output.accessPtr(y,width-1-x);
                const T* src = getPtr(x,y);
                for(int i = 0 ; i < STEP ; i++){
                    *(dest + i) = *(src + i);
                }
            }
        }
        return  output;
    }
    Data2D<T,STEP> rotate90CW(){
        Data2D<T,STEP> output(height, width);
        for(int y = 0; y < height ; y++){
            for(int x = 0; x < width ; x++){
                T* dest = output.accessPtr(height-1-y,x);
                const T* src = getPtr(x,y);
                for(int i = 0 ; i < STEP ; i++){
                    *(dest + i) = *(src + i);
                }
            }
        }
        return  output;
    }
    Data2D<T,STEP> scaleBilinear(int w2, int h2) const {
        Data2D<T,STEP> temp(w2,h2);
        int w = getWidth();
        int h = getHeight();
        int a, b, c, d, x, y, index ;
        FP x_ratio = ((FP)(w-1))/w2 ;
        FP y_ratio = ((FP)(h-1))/h2 ;
        FP x_diff, y_diff, blue, red, green ;
        int offset = 0 ;
        auto it = temp.begin();
        for (int i=0;i<h2;i++) {
            for (int j=0;j<w2;j++) {
                x = (int)(x_ratio * j) ;
                y = (int)(y_ratio * i) ;
                x_diff = (x_ratio * j) - x ;
                y_diff = (y_ratio * i) - y ;
                index = (y*w+x) ;
                auto a = getPtr(x,y);
                auto b = getPtr(x+1,y);
                auto c = getPtr(x,y+1);
                auto d = getPtr(x+1,y+1);
                for(int n = 0 ; n < STEP; n++){
                    *(it++) = (*a++)*(1-x_diff)*(1-y_diff) + (*b++)*(x_diff)*(1-y_diff) +
                    (*c++)*(y_diff)*(1-x_diff)   + (*d++)*(x_diff*y_diff);
                }
            }
        }
        return temp ;
    }
};

template<typename T>
class ImageRGBA : public Data2D<T, 4>{
    public:
    ImageRGBA() : Data2D<T, 4>(){
    }
    ImageRGBA( std::vector<T>& data_, int width_) : Data2D<T, 4>(data_, width_){
    }
    ImageRGBA( const std::vector<T>& data_, int width_) : Data2D<T, 4>(data_, width_){
    }
    ImageRGBA( std::vector<T>&& data_, int width_) : Data2D<T, 4>(data_, width_){
    }
    ImageRGBA( int width_, int height_) : Data2D<T, 4>(width_, height_){
    }
    ImageRGBA& operator=( ImageRGBA&& other) = default;
    ImageRGBA(const ImageRGBA& other) = default;

    inline FP pix_ABSdiff(const T* positive, const T* negative) const {
        FP diff_red =   static_cast<FP>( *(positive++) ) - *(negative++);
        FP diff_green = static_cast<FP>( *(positive++) ) - *(negative++);
        FP diff_blue =  static_cast<FP>( *(positive++) ) - *(negative++);
        return ( std::abs(diff_red) + std::abs(diff_green) + std::abs(diff_blue) )/(3*255);
    }
    Data2D<FP> calculate_dx() const {
        Data2D<FP> output(this->width, this->height);
        auto it = output.begin();
        for(int y = 0 ; y < this->height; y++){
            //handle left edge
            *(it++) = pix_ABSdiff( this->getPtr(0,y), this->getPtr(1,y) );
            //handle inner pixels
            for(int x = 1 ; x < this->width - 1; x++){
                auto leftPix = this->getPtr(x-1,y);
                auto rightPix = this->getPtr(x+1,y);
                *(it++) = pix_ABSdiff(rightPix, leftPix);
            }
            //handle right edge
            *(it++) = pix_ABSdiff(this->getPtr(this->width-1,y) , this->getPtr(this->width-1,y));
        }
        return output;
    }
    Data2D<FP> calculate_dy() const {
        Data2D<FP> output(this->width, this->height);
        auto it = output.begin();
        //handle top edge
        for(int x = 0 ; x < this->width; x++){
            auto top = this->getPtr(x,0);
            auto bottom = this->getPtr(x,1);
            *(it++) = pix_ABSdiff(bottom, top);
        }
        //handle inner pixels
        for(int y = 1 ; y < this->height-1; y++){
            for(int x = 0 ; x < this->width; x++){
                auto top = this->getPtr(x,y-1);
                auto bottom = this->getPtr(x,y+1);
                *(it++) = pix_ABSdiff(bottom, top);
            }
        }
        //handle bottom edge
        for(int x = 0 ; x < this->width; x++){
            auto top = this->getPtr(x,this->height-2);
            auto bottom = this->getPtr(x,this->height-1);
            *(it++) = pix_ABSdiff(bottom, top);
        }
        return output;
    }
    Data2D<FP> calculateEnergy(const ImageRGBA<unsigned char>& mask) const {
        auto dx = calculate_dx();
        auto dy = calculate_dy();
        Data2D<FP> output(this->width, this->height);
        for(int i = 0 ; i < dx.size() ; i++){
            auto ptr = mask.getPtr(i);
            FP lowEnergy = *(ptr++) / (FP)255;
            FP highEnergy = *(ptr++) / (FP)255;
            output[i] = ((dx[i]*4 + dy[i]) / 5) + 5 + 5*(highEnergy-lowEnergy) ;
        }
        return output;
    }
    Data2D<FP> calculateEnergy() const {
        auto dx = calculate_dx();
        auto dy = calculate_dy();
        Data2D<FP> output(this->width, this->height);
        for(int i = 0 ; i < dx.size() ; i++){
            output[i] = ((dx[i]*4 + dy[i]) / 5) ;
        }
        return output;
    }
};