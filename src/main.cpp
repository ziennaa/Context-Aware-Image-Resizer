#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h"
#include <iostream>
using namespace std;
int main(){
    int w, h, channels;
    // image width, height and how many color values per pixel
    // stb_load will fill it for you
    unsigned char* img = stbi_load("images/test.png", &w, &h, &channels, 3);
    // stb_load opens the image decodes the compressed file
    // fill w , h, channels with actual values
    // the last argeuemtn 3 forces 3rgb
    // pixel val : 0 to 255 therefore unsigned char
    if(!img){
        cout<<"Failed to load\n";
        return 1;
    }
    cout<<"Loaded: "<<w<<"x"<<h<<"\n";
    stbi_write_png("output.png", w, h, 3, img, w*3);
    stbi_image_free(img);
    return 0;
}