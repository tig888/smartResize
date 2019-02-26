#pragma once
#include<vector>
#include<algorithm>
#include<limits>
#include<map>
#include <chrono>

#include"Image.hpp"


struct Point{
	int x,y;
};

class Timer{
	decltype(std::chrono::steady_clock::now()) start;
	public:
	void tick(){
		start = std::chrono::steady_clock::now();
	}
	void tock(const char* str){
		std::cout << str << " took : "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
		<< " ms" << std::endl;
	}
	int tock(){
		return  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
	}
	void ticktock(const char* str){
		std::cout << str << " took : "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
		<< " ms" << std::endl;
		start = std::chrono::steady_clock::now();
	}
};


template<int step = 1>
class IdxD2{
	int width, height;
	public:
	IdxD2(int width, int height) : width(width), height(height){
	}

	inline int  size() { return width * height * step;}
    inline int  rows() { return height;}
    inline int  cols() { return width ;}
	inline int  below(int idx) { return idx + width * step;}
	inline int  above(int idx) { return idx - width * step;}
	inline int  right(int idx) { return idx + step;}
	inline int  left(int idx) { return idx - step;}
	inline int  point2idx(int x, int y) { return (y * width + x) * step;}

	inline bool leftEdge(int idx) { return (idx/step) % width == 0;}
	inline bool rightEdge(int idx) { return ((idx/step) + 1) % width == 0;}
	inline bool bottomEdge(int idx) { return (idx/step) >= width * (height - 1);}
    inline bool topEdge(int idx) { return (idx/step) < width;}

	inline bool rightEdge(int x, int y) { return rightEdge(point2idx(x,y)) ;}
	inline bool leftEdge(int x, int y) { return leftEdge(point2idx(x,y)) ;}
	inline bool bottomEdge(int x, int y) { return bottomEdge(point2idx(x,y)) ;}
	inline bool topEdge(int x, int y) { return topEdge(point2idx(x,y)) ;}

    inline bool edge(int idx) { return leftEdge(idx) || rightEdge(idx) || bottomEdge(idx) || topEdge(idx);}
	inline int getX(int idx) { return (idx/step) % width;}
	inline int getY(int idx) { return (idx/step) / width;}

};




class Seam{
    struct Node{
        int trace;
        double energy;
        int hashIdx;
        Node(int a, double b, int c) : trace(a), energy(b), hashIdx(c){};
		Node() : trace(0), energy(0), hashIdx(0){};
    };
    struct TraceLinkage{
        int idx, parentIdx;
    };
    std::multimap<double, TraceLinkage> costs;
    Data2D<Node> workspace;
    const ImageRGBA<unsigned char>& originalImage;
	const ImageRGBA<unsigned char>& originalMask;

	Timer timer;

	public:
	Seam(const ImageRGBA<unsigned char>& image, const ImageRGBA<unsigned char>& mask);

	inline void addPotentialNewPoint(int idx, double e, int parentIdx);
    void initializeRun();
	void checkSeam(std::set<int>& seam);
    void reduceWorkspace(std::set<int> seam);
	void expandWorkspace(std::set<int>& seam, int N);
	std::set<int> findSeam();
	std::set<int> findOneSeam();


	void redistributeEnergy(const std::set<int>&  path);
	void adjustEnergy(const std::set<int>&  path);
    void fixEnergies(std::vector<Point> nodes);
    std::vector<Point> getNodesThatNeedEnergyFix(const std::set<int>&  path);
	void debugSeam(std::set<int>& path);
	std::vector<int> removeNSeams(int N);
	ImageRGBA<unsigned char> resultImage();
	ImageRGBA<unsigned char> resultMask();
	std::set<int> addNSeams(int N);
};

