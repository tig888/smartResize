#include<iostream>
#include<vector>
#include<lodepng.h>
#include<iomanip>

#include <smartResize.hpp>
using namespace std;

void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height) {
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

void printHelp(){
	cout << "Usage example:" << endl;
    cout << "   resize inpute.png output.png 'width' 'height' <mask.png>" << endl;
    cout << "\"inpute.png\" path to png input inage, at least 10x10 pixels" << endl;
    cout << "\"output.png\" name of png result inage" << endl;
    cout << "'width' integer higher then 10 - desired image width" << endl;
    cout << "'height' integer higher then 10  - desired image width" << endl;

    cout << "\"mask.png\" optional mask image " << endl;
}

int main(int argc, char *argv[]){
	if(argc < 5){
		printHelp();
		return 1;
	}
	const char* inputImageFileName = argv[1];
	const char* outputImageFileName = argv[2];
	const char* maskImageFileName = nullptr;
	if(argc == 6){
		maskImageFileName = argv[5];
	}

	std::vector<unsigned char> sourceImage; //raw pixels
	std::vector<unsigned char> mask;
	unsigned imageWidth, imageHeight;
	unsigned maskWidth, maskHeight;


	unsigned error = lodepng::decode(sourceImage, imageWidth, imageHeight, inputImageFileName);
	if(error) {
		cout << "Could not read image file" << endl;
		std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		return 1;
	}
	if(maskImageFileName != nullptr){
		error = lodepng::decode(mask, maskWidth, maskHeight, maskImageFileName);
		if(error) {
			cout << "Could not read mask file" << endl;
			std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
			return 1;
		}
	}

	int desiredWidth, desiredHeight;

	std::string arg(argv[3]);
	try {
		std::size_t pos;
		desiredWidth = std::stoi(arg, &pos);
		if (pos < arg.size()) {
			std::cerr << "Trailing characters after number: " << arg << '\n';
		}
	} catch (std::invalid_argument const &ex) {
		std::cerr << "Invalid number: " << arg << '\n';
		return 1;
	} catch (std::out_of_range const &ex) {
		std::cerr << "Number out of range: " << arg << '\n';
		return 1;
	}
	std::string arg2(argv[4]);
	try {
		std::size_t pos;
		desiredHeight = std::stoi(arg2, &pos);
		if (pos < arg2.size()) {
			std::cerr << "Trailing characters after number: " << arg2 << '\n';
		}
	} catch (std::invalid_argument const &ex) {
		std::cerr << "Invalid number: " << arg2 << '\n';
		return 1;
	} catch (std::out_of_range const &ex) {
		std::cerr << "Number out of range: " << arg2 << '\n';
		return 1;
	}
    if(desiredWidth < 10 || desiredHeight < 10){
        printHelp();
        return 1;
    }
    if(imageHeight < 10 || imageWidth < 10){
        printHelp();
        return 1;
    }

    ///////////////////////////////////////////////////////////////
    std::vector<unsigned char> output;
    if(maskImageFileName != nullptr){
	    output = smartResize(&sourceImage, imageWidth, desiredWidth, desiredHeight, &mask);
    }else{
        output = smartResize(&sourceImage, imageWidth, desiredWidth, desiredHeight);
    }
    if(output.size() > 0) encodeOneStep(outputImageFileName, output, desiredWidth, desiredHeight);

	return 0;
}