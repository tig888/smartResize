#include<iostream>
#include<set>
#include<vector>

#include<Seam.hpp>
#include<lodepng.h>
#include<Image.hpp>

using namespace std;

void saveToFile(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height) {
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

void saveToFile(const char* filename, std::vector<double>& in, unsigned width, unsigned height) {
	vector<unsigned char> out;
	out.reserve(in.size() * 4);
	for(auto a : in){
		if(a >= 0){
			out.push_back(a*255);
			out.push_back(0);
		} else {
			out.push_back(0);
			out.push_back(-a*255);
		}
		out.push_back(0);
		out.push_back(255);
	}
	saveToFile( filename,  out,  width,  height);
}


Seam::Seam(const ImageRGBA<unsigned char>& image, const ImageRGBA<unsigned char>& mask) :
originalImage(image),
originalMask(mask),
workspace(image.getWidth(), image.getHeight())
{
    auto it = workspace.begin();
    Data2D<double> energy;
    if(mask.size() > 0 ){
        energy = originalImage.calculateEnergy(mask);
    }
    else energy = originalImage.calculateEnergy();
    IdxD2<> dim(energy.getWidth(), energy.getHeight());
    for(int x = 0 ; x < dim.cols() ; x++){
        int idx = dim.point2idx(x,0);
        *(it++) = Node{-1,energy[idx],idx};
    }
    for(int y = 1 ; y < dim.rows() ; y++){
        for(int x = 0 ; x < dim.cols() ; x++){
            int idx = dim.point2idx(x,y);
            *(it++) = Node{0,energy[idx],idx};
        }
    }
}
inline void Seam::addPotentialNewPoint(int idx, double e, int parentIdx){
    if(idx > workspace.size())
    {
        IdxD2<> O(workspace.getWidth(), workspace.getHeight());
        cout << "Error addPotentialNewPoint: idx(" << O.getX(idx) << ":" << O.getY(idx) << ") out of workspace" << endl;
        exit (EXIT_FAILURE);
    }
    if (workspace[idx].trace < 0){
        workspace[idx].trace = parentIdx;
        costs.emplace(e + workspace[idx].energy, TraceLinkage{idx, parentIdx}) ;
    }
}

void Seam::initializeRun(){
    costs.clear();
    for(int x = 0 ; x < workspace.getWidth(); x++){
        double e = workspace[x].energy;
        costs.emplace(e, TraceLinkage{x, -1});
    }
    IdxD2<> dim(workspace.getWidth(), workspace.getHeight());
    for(int y = 0 ; y < workspace.getHeight() ; y++){
        for(int x = 0 ; x < workspace.getWidth() ; x++){
            int idx = dim.point2idx(x,y);
            workspace[idx].trace = -1;
        }
    }
}

void Seam::checkSeam(set<int>& seam){
    IdxD2<> O(workspace.getWidth(), workspace.getHeight());
    if(seam.size() != workspace.getHeight())
    {
        for( auto p : seam){
            cout << O.getX(p) << " " << O.getY(p) << endl;
        }
        cout << "Error reduceWorkspace: seam size " << seam.size() <<  " != workspace height "<< workspace.getHeight() << endl;
        exit (EXIT_FAILURE);
    }
    int y = 0;
    for(auto idx : seam){
        if(O.getY(idx) != y++){
            cout << "Error reduceWorkspace: not exactly one point per row" << endl;
            exit (EXIT_FAILURE);
        }
    }
}

void Seam::reduceWorkspace(set<int> seam){
    checkSeam(seam);
    Data2D<Node> newWorkspace(workspace.getWidth()-1, workspace.getHeight());
    auto ommit = seam.begin();
    auto newIt = newWorkspace.begin();
    for(int i = 0 ; i < newWorkspace.size() ; i++)
    {
        if(i == *ommit) {
            ommit++;
            if(ommit == seam.end()) ommit = seam.begin();
            continue;
        }
        *(newIt++) = workspace[i];
    }
    workspace = move(newWorkspace);
}

void Seam::expandWorkspace(set<int>& seam, int N){
    checkSeam(seam);
    Data2D<Node> newWorkspace(workspace.getWidth()+N, workspace.getHeight());
    auto copyInto = newWorkspace.begin();
    auto copyFrom = workspace.begin();
    auto copy = seam.begin();
    while(copyFrom != workspace.end()){
        if(copyFrom - workspace.begin() == *copy )
        {
            for(int i = 0 ; i < N ; i++){
                *(copyInto++) = *copyFrom;
            }
            copy++;
            if(copy == seam.end()){
                copy = seam.begin(); //reset
            }
        }
        *(copyInto++) = *(copyFrom++);
    }
    workspace = move(newWorkspace);
}

set<int> Seam::findSeam(){
    pair<double,TraceLinkage> node;
    IdxD2<> O(workspace.getWidth(), workspace.getHeight());

    node = *(costs.begin());
    costs.erase(costs.begin());
    int idx = node.second.idx;
    int e = node.first;
    while( !O.bottomEdge(idx) ){
        if(!O.leftEdge(idx)){
            addPotentialNewPoint(O.below(O.left(idx)), e, idx);
        }
        if(!O.rightEdge(idx)){
            addPotentialNewPoint(O.below(O.right(idx)), e, idx);
        }
        addPotentialNewPoint(O.below(idx), e, idx);
        node = *(costs.begin());
        costs.erase(costs.begin());
        idx = node.second.idx;
        e = node.first;
    }
    set<int> path;
    path.insert(idx);
    int parent = workspace[node.second.idx].trace;
    while(parent >= 0){
        path.insert(parent);
        parent = workspace[parent].trace;
    }
    if(path.size() != workspace.getHeight())
    {
        for( auto p : path){
            cout << O.getX(p) << " " << O.getY(p) << endl;
        }
        cout << "Error findSeam: seam size " << path.size() <<  " != workspace height "<< workspace.getHeight() << endl;
        debugSeam(path);
        exit (EXIT_FAILURE);
    }
    return path;
}

set<int> Seam::findOneSeam(){
    initializeRun();
    return findSeam();
}


void Seam::redistributeEnergy(const set<int>&  path){
    IdxD2<> O(workspace.getWidth(), workspace.getHeight());
    for(auto idx : path){
        if(!O.leftEdge(idx)){
            workspace[O.left(idx)].energy += workspace[idx].energy/3;
        }
        if(!O.rightEdge(idx)){
            workspace[O.right(idx)].energy += workspace[idx].energy/3;
        }
    }
}

void Seam::adjustEnergy(const set<int>&  path){
    IdxD2<> O(workspace.getWidth(), workspace.getHeight());
    for(auto idx : path){
        workspace[idx].energy += 0.1;
        workspace[idx].energy *= 1.1;
        if(!O.leftEdge(idx)){
            workspace[O.left(idx)].energy *= 1.1;
        }
        if(!O.rightEdge(idx)){
            workspace[O.right(idx)].energy *= 1.1;
        }
    }
}

void Seam::fixEnergies(vector<Point> nodes){
    IdxD2<> O(workspace.getWidth(), workspace.getHeight());
    //cout << "Energy fixes &&&&&&&&&&&&&&&&" << endl;
    for(auto n : nodes){
        int x = n.x;
        int y = n.y;
        int origIdxMiddle = workspace[O.point2idx(x, y)].hashIdx;
        int origIdxLeft 	= origIdxMiddle;
        int origIdxRight 	= origIdxMiddle;
        int origIdxTop 		= origIdxMiddle;
        int origIdxBottom 	= origIdxMiddle;
        if(!O.leftEdge(x, y)) 		origIdxLeft   = workspace[O.point2idx(x-1, y)].hashIdx;
        if(!O.rightEdge(x, y)) 		origIdxRight  = workspace[O.point2idx(x+1, y)].hashIdx;
        if(!O.topEdge(x, y)) 		origIdxTop    = workspace[O.point2idx(x, y-1)].hashIdx;
        if(!O.bottomEdge(x, y)) 	origIdxBottom = workspace[O.point2idx(x, y+1)].hashIdx;
        //cout << "     " << O.getX(origIdxLeft) << "  " << O.getX(origIdxMiddle) << "     ";
        double dx = originalImage.pix_ABSdiff(originalImage.getPtr(origIdxLeft), originalImage.getPtr(origIdxRight));
        double dy = originalImage.pix_ABSdiff(originalImage.getPtr(origIdxTop), originalImage.getPtr(origIdxBottom));
        double newEnergy = (dx+dy)/2;
        //cout << "Old: " << workspace[O.point2idx(x, y)].energy << "  New: " << newEnergy << endl;
        workspace[O.point2idx(x, y)].energy = newEnergy;
    }
}

vector<Point> Seam::getNodesThatNeedEnergyFix(const set<int>&  path){
    IdxD2<> O(workspace.getWidth(), workspace.getHeight());
    vector<Point> nodes;
    int y = 0;
    for(auto idx : path){
        if(!O.leftEdge(idx)){
            Point p;
            p.x = O.getX(O.left(idx));
            p.y = y;
            nodes.push_back(p);
        }
        if(!O.rightEdge(idx)){
            Point p;
            p.x = O.getX(idx);
            p.y = y;
            nodes.push_back(p);
        }
        y++;
    }
    return nodes;
}

void Seam::debugSeam(set<int>& path){
    vector<unsigned char> temp;
    int i = 0;
    for(auto w : workspace){
        auto a = w.trace;
        if(path.find(i)!= path.end()){
            temp.push_back(0);
            temp.push_back(0);
            temp.push_back(255);
        }
        else if(a > 0){
            temp.push_back(255);
            temp.push_back(0);
            temp.push_back(0);
        } else if (a < 0) {
            temp.push_back(255);
            temp.push_back(255);
            temp.push_back(0);
        }else {
            temp.push_back(0);
            temp.push_back(255);
            temp.push_back(0);
        }

        temp.push_back(255);
        i++;
    }
    saveToFile("trace.png", temp, workspace.getWidth(), workspace.getHeight());
}

vector<int> Seam::removeNSeams(int N){
    set<int>  path;
    int initTime = 0, findTime = 0, adjustTime = 0, expandTime = 0;
    for(int i = 0 ; i < N ; i++)
    {
        timer.tick();
        initializeRun();
        initTime += timer.tock();

        timer.tick();
        path = findSeam();
        findTime += timer.tock();

        timer.tick();
        redistributeEnergy(path);
        adjustTime += timer.tock();

        timer.tick();
        reduceWorkspace(path);
        expandTime += timer.tock();
    }
    cout << "addNSeams initializing took: " << initTime << " ms" << endl;
    cout << "addNSeams finding took: " << findTime << " ms" << endl;
    cout << "addNSeams adjusting took: " << adjustTime << " ms" << endl;
    cout << "addNSeams reduce took: " << expandTime << " ms" << endl;
    // for(int i = 0 ; i < N ; i++)
    // {
    //     initializeRun();
    //     //costs.emplace(energy.getVal(450,0), TraceLinkage{450, -1});
    //     path = findSeam();
    // 	redistributeEnergy(path);
    // 	auto needFix = getNodesThatNeedEnergyFix(path);
    //     //cout << path.size() << endl;
    //     reduceWorkspace(path);
    // 	//fixEnergies(needFix);
    // }
    // Data2D<double> newEnergy(width, height);
    // auto it = newEnergy.begin();
    // for(auto w : workspace){
    // 	*(it++) = w.energy;
    // }
    // encodeOneStep("afterEnergy.png", newEnergy, newEnergy.getWidth(), newEnergy.getHeight());


    vector<int> remainingPixels;
    remainingPixels.reserve(workspace.size());
    for(auto n : workspace){
        if ( n.hashIdx >= 0) remainingPixels.emplace_back(n.hashIdx);
    }
    auto it = remainingPixels.begin();
    for(int i = 0 ; i < 1000 ; i++)
    {
        if(*(it) != i){
            //cout << "removed " << *(it) << endl;
        }else it++;
    }
    auto itt = remainingPixels.begin();
    ImageRGBA<unsigned char> cp(originalImage);
    for(int i = 0 ; i < cp.size()/4 ; i++)
    {
        if(*(itt) != i){
            //cout << "removed " << *(it) << endl;
            auto ptr = cp.data() + i*4;
            *(ptr++) = 0;
            *(ptr++) = 0;
            *(ptr++) = 0;
        }else itt++;
    }
    saveToFile("removedPixels.png", cp, cp.getWidth(), cp.getHeight());

    return remainingPixels;
}


ImageRGBA<unsigned char> Seam::resultImage(){
    ImageRGBA<unsigned char> output(workspace.getWidth(), workspace.getHeight());
    auto ptrDest = output.accessPtr(0);
    for(auto n : workspace){
        auto ptrSrc = originalImage.getPtr(n.hashIdx);
        for(int i = 0 ; i < 4 ; i++){
            *(ptrDest++) = *(ptrSrc++);
        }
    }
    return output;
}
ImageRGBA<unsigned char> Seam::resultMask(){
    ImageRGBA<unsigned char> output(workspace.getWidth(), workspace.getHeight());
    auto ptrDest = output.accessPtr(0);
    for(auto n : workspace){
        auto ptrSrc = originalMask.getPtr(n.hashIdx);
        for(int i = 0 ; i < 4 ; i++){
            *(ptrDest++) = *(ptrSrc++);
        }
    }
    return output;
}
set<int> Seam::addNSeams(int N){
    set<int> path;
    int initTime = 0, findTime = 0, adjustTime = 0, expandTime = 0;
    for(int i = 0 ; i < N ; i++)
    {
        timer.tick();
        initializeRun();
        initTime += timer.tock();

        timer.tick();
        path = findSeam();
        findTime += timer.tock();

        timer.tick();
        adjustEnergy(path);
        adjustTime += timer.tock();

        timer.tick();
        expandWorkspace(path, 1);
        expandTime += timer.tock();
    }
    cout << "addNSeams initializing took: " << initTime << " ms" << endl;
    cout << "addNSeams finding took: " << findTime << " ms" << endl;
    cout << "addNSeams adjusting took: " << adjustTime << " ms" << endl;
    cout << "addNSeams expanding took: " << expandTime << " ms" << endl;

    return path;
}