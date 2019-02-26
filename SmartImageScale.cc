#include"SmartImageScale.hpp"
#include"Seam.hpp"
#include"Image.hpp"

using namespace std;

SmartImageScale::SmartImageScale(const std::vector<unsigned char>& input, int width) :
	origImage(input, width),
	workImage(input, width){}

SmartImageScale::SmartImageScale(const std::vector<unsigned char>& input, const std::vector<unsigned char>& mask, int width) :
	origImage(input, width),
	origMask(mask, width),
	workMask(mask, width),
	workImage(input, width){}

auto SmartImageScale::verticalAdjust(int N) -> void{
	Timer t;
	if( N == 0 ) return;
	t.tick();
	cout << "	adjusting " << N << " vertical seams" << endl;
	Seam seam(workImage, workMask);
	t.ticktock("SIS Seam Constructor");
	if( N > 0){
		seam.addNSeams(N);
		t.ticktock("SIS Seam adding");
	} else{
		seam.removeNSeams(-N);
		t.ticktock("SIS Seam removeing");
	}
	workImage = seam.resultImage();
	if(workMask.size() > 0) workMask = seam.resultMask();
	t.ticktock("SIS Seam taking result");
	cout << workImage.getWidth() << "x" << workImage.getHeight() << endl;
}
auto SmartImageScale::horizontalAdjust(int N) -> void{
	Timer t;
	if( N == 0 ) return;
	t.tick();
	cout << "	adjusting " << N << " horisontal seams" << endl;
	workImage = static_cast<ImageRGBA<unsigned char>&& >(workImage.rotate90CCW());
	if(workMask.size() > 0) workMask = static_cast<ImageRGBA<unsigned char>&& >(workMask.rotate90CCW());
	t.ticktock("SIS rotatingCCW");
	cout << workImage.getWidth() << "x" << workImage.getHeight() << endl;
	Seam seam(workImage, workMask);
	t.ticktock("SIS Seam Constructor");
	if( N > 0){
		seam.addNSeams(N);
		t.ticktock("SIS Seam adding");
	} else{
		seam.removeNSeams(-N);
		t.ticktock("SIS Seam removeing");
	}
	workImage = seam.resultImage();
	if(workMask.size() > 0) workMask = seam.resultMask();
	t.ticktock("SIS Seam taking result");
	cout << workImage.getWidth() << "x" << workImage.getHeight() << endl;
	workImage = static_cast<ImageRGBA<unsigned char>&& >(workImage.rotate90CW());
	if(workMask.size() > 0) workMask = static_cast<ImageRGBA<unsigned char>&& >(workMask.rotate90CW());
	t.ticktock("SIS rotatingCCW");
	cout << workImage.getWidth() << "x" << workImage.getHeight() << endl;
}


auto SmartImageScale::scale(int desiredWidth, int desiredHeight) -> ImageRGBA<unsigned char> {
	Timer t;
	cout << "SIS SmartScaling image from " << origImage.getWidth() << 'x' << origImage.getHeight();
	cout << " to " << desiredWidth << 'x' << desiredHeight << endl;
	FP originalAspectRatio = ((FP)origImage.getWidth()) / origImage.getHeight();
	FP desiredAspectRatio = ((FP)desiredWidth) / desiredHeight;
	t.tick();
	if(desiredWidth == origImage.getWidth() && desiredHeight == origImage.getHeight()){

	}
	else if(desiredWidth >= origImage.getWidth() && desiredHeight >= origImage.getHeight()){
		//expanding image
		int w, h;
		if(originalAspectRatio > desiredAspectRatio){
			//scale to match the desired width
			w = desiredWidth;
			h = desiredWidth/originalAspectRatio;
		} else{
			//scale to match the desired height
			w = desiredHeight*originalAspectRatio;
			h = desiredHeight;
		}
		cout << "SIS Bilinear expanding to " << w << 'x' << h << endl;
		workImage = static_cast<ImageRGBA<unsigned char>&& >(workImage.scaleBilinear(w, h));
		if(workMask.size() > 0) workMask = static_cast<ImageRGBA<unsigned char>&& >(workMask.scaleBilinear(w, h));
		cout << workImage.getWidth() << "x" << workImage.getHeight() << endl;
		t.ticktock("SIS Bilinear scaleing ");

		verticalAdjust(desiredWidth - w); //add
		horizontalAdjust(desiredHeight - h); //add

	} else if(desiredWidth <= origImage.getWidth() && desiredHeight <= origImage.getHeight()){
		int w, h;
		if(originalAspectRatio < desiredAspectRatio){
			//scale to match the desired width
			w = desiredWidth;
			h = desiredWidth/originalAspectRatio;
		} else{
			//scale to match the desired height
			w = desiredHeight*originalAspectRatio;
			h = desiredHeight;
		}
		cout << "SIS Bilinear shrinking to " << w << 'x' << h << endl;
		workImage = static_cast<ImageRGBA<unsigned char>&& >(workImage.scaleBilinear(w, h));
		if(workMask.size() > 0) workMask = static_cast<ImageRGBA<unsigned char>&& >(workMask.scaleBilinear(w, h));

		cout << workImage.getWidth() << "x" << workImage.getHeight() << endl;
		t.ticktock("SIS Bilinear scaleing ");
		verticalAdjust(desiredWidth - w); //remove
		horizontalAdjust(desiredHeight - h); //remove
	} else if(desiredWidth < origImage.getWidth() && desiredHeight > origImage.getHeight()){
		verticalAdjust(desiredWidth - origImage.getWidth()); //remove
		horizontalAdjust(desiredHeight - origImage.getHeight()); //add
	} else if(desiredWidth > origImage.getWidth() && desiredHeight < origImage.getHeight()){
		horizontalAdjust(desiredHeight - origImage.getHeight()); //add
		verticalAdjust(desiredWidth - origImage.getWidth()); //remove
	}
	return workImage;
}
