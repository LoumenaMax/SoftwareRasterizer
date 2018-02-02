/* Release code for program 1 CPE 471 Fall 2016 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

int g_width, g_height, mode;

typedef struct coord{
	float x;
	float y;
	float z;
	coord(){};
} coord;

typedef struct bounding_box{
  float ymin;
  float ymax;
  float xmin;
  float xmax;
  coord v1;
  coord v2;
  coord v3;
  bool top;
  bounding_box(){};
} bounding_box;

/*
   Helper function you will want all quarter
   Given a vector of shapes which has already been read from an obj file
   resize all vertices to the range [-1, 1]
 */
void resize_obj(std::vector<tinyobj::shape_t> &shapes){
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
         if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

         if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
         if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

         if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
         if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
      }
   }

	//From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
         assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
         assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
         assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
      }
   }
}

void create_box(bounding_box *newBox, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
	newBox->xmin = min(min(x1, x2), x3);
	newBox->xmax = max(max(x1, x2), x3);
	newBox->ymin = min(min(y1, y2), y3);
	newBox->ymax = max(max(y1, y2), y3);
	newBox->v1.x = x1;
	newBox->v1.y = y1;
	newBox->v1.z = z1;
	newBox->v2.x = x2;
	newBox->v2.y = y2;
	newBox->v2.z = z2;
	newBox->v3.x = x3;
	newBox->v3.y = y3;
	newBox->v3.z = z3;
}

void transform_box(bounding_box *box, float x_offset, float y_offset) {
	box->xmin += x_offset;
	box->xmax += x_offset;
	box->v1.x += x_offset;
	box->v2.x += x_offset;
	box->v3.x += x_offset;
	box->ymin += y_offset;
	box->ymax += y_offset;
	box->v1.y += y_offset;
	box->v2.y += y_offset;
	box->v3.y += y_offset;
}

void resize_box(bounding_box *box, int width, int height) {
	float resize_ratio = ((float)width / (box->xmax - box->xmin));
	box->xmin *= resize_ratio;
	box->xmax *= resize_ratio;
	box->ymin *= resize_ratio;
	box->ymax *= resize_ratio;
	box->v1.x *= resize_ratio;
	box->v1.y *= resize_ratio;
	box->v2.x *= resize_ratio;
	box->v2.y *= resize_ratio;
	box->v3.x *= resize_ratio;
	box->v3.y *= resize_ratio;
	if((box->ymax - box->ymin) > height) {
		resize_ratio = ((float)height / (box->ymax - box->ymin));
		box->xmin *= resize_ratio;
		box->xmax *= resize_ratio;
		box->ymin *= resize_ratio;
		box->ymax *= resize_ratio;
		box->v1.x *= resize_ratio;
		box->v1.y *= resize_ratio;
		box->v2.x *= resize_ratio;
		box->v2.y *= resize_ratio;
		box->v3.x *= resize_ratio;
		box->v3.y *= resize_ratio;
		box->top *= true;
	} else {
		box->top = false;
	}
}
void resize_box(bounding_box *box, float resize_ratio) {
	box->xmin *= resize_ratio;
	box->xmax *= resize_ratio;
	box->ymin *= resize_ratio;
	box->ymax *= resize_ratio;
	box->v1.x *= resize_ratio;
	box->v1.y *= resize_ratio;
	box->v2.x *= resize_ratio;
	box->v2.y *= resize_ratio;
	box->v3.x *= resize_ratio;
	box->v3.y *= resize_ratio;
}

void bary_coords(float x, float y, bounding_box box, coord *bary) {
	float x1 = box.v1.x;
	float y1 = box.v1.y;
	float x2 = box.v2.x;
	float y2 = box.v2.y;
	float x3 = box.v3.x;
	float y3 = box.v3.y;
	float detT = (((y2-y3)*(x1-x3)) + ((x3-x2)*(y1-y3)));
	bary->x = (((y2-y3)*(x-x3)) + ((x3-x2)*(y-y3))) / detT;
	bary->y = (((y3-y1)*(x-x3)) + ((x1-x3)*(y-y3))) / detT;
	bary->z = 1.0 - bary->x - bary->y;
}

void mode1(int x, int y, bounding_box box, coord bary, std::shared_ptr<Image> img, unsigned short *zbuf) {
	unsigned short colour = 255 * ((((box.v1.z + 1.0)/2) * bary.x) + (((box.v2.z + 1.0)/2) * bary.y) + (((box.v3.z + 1.0)/2) * bary.z));
	if(colour > zbuf[(x * g_height) + y]) {
		zbuf[(x * g_height) + y] = colour;
		img->setPixel(x, y, (int)colour, 0, 0);
	}
}

void write_coordinates(bounding_box box, bounding_box *img_box, std::shared_ptr<Image> img, coord *bary, unsigned short *zbuf) {
	for(int j = box.ymin; j < box.ymax; j++) {
		for(int i = box.xmin; i < box.xmax; i++) {
			bary_coords(((float)i), ((float)j), box, bary);
			if((0.0 <= bary->x  && bary->x <= 1.0) && (0.0 <= bary->y && bary->y <= 1.0) && (0.0 <= bary->z && bary->z <= 1.0)) {
				if(mode == 1) {
					mode1(i, j, box, *bary, img, zbuf);
				}
				if(mode == 2) {
					img->setPixel(i, j, 0, 255, 0);
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	if(argc < 6) {
		cout << "Usage: Assignment1 meshfile imagefile <width> <height> <mode>" << endl;
		return 0;
	}
	// OBJ filename
	string meshName(argv[1]);
	string imgName(argv[2]);

	//set g_width and g_height appropriately!
	try {
  		g_width = stoi(argv[3]);
		g_height = stoi(argv[4]);
		mode = stoi(argv[5]);
	}
	catch(...) {
		cout << "Usage: Assignment1 meshfile imagefile <width> <height> <mode>" << endl;
		return 0;
	}

	//g_width = g_height = 100;

   	//create an image
	auto image = std::make_shared<Image>(g_width, g_height);

	// triangle buffer
	vector<unsigned int> triBuf;
	// position buffer
	vector<float> posBuf;
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes; // geometry
	vector<tinyobj::material_t> objMaterials; // material
	string errStr;
	
	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	/* error checking on read */
	if(!rc) {
		cerr << errStr << endl;
	} else {
		posBuf = shapes[0].mesh.positions;
		triBuf = shapes[0].mesh.indices;
	}
	cout << "Number of vertices: " << posBuf.size()/3 << endl;
	cout << "Number of triangles: " << triBuf.size()/3 << endl;

 	//keep this code to resize your object to be within -1 -> 1
	resize_obj(shapes); 

	//TODO add code to iterate through each triangle and rasterize it 
	
	bounding_box *img_box = new bounding_box();
	img_box->ymin = 1.1754E+38F;
	img_box->ymax = -1.1754E+38F;
	img_box->xmin = 1.1754E+38F;
	img_box->xmax = -1.1754E+38F;
	for( auto i = shapes[0].mesh.positions.begin(); i != shapes[0].mesh.positions.end(); i++) {
		img_box->xmax = max(float(*i), img_box->xmax);
		img_box->xmin = min(float(*i), img_box->xmin);
		i++;
		img_box->ymax = max(float(*i), img_box->ymax);
		img_box->ymin = min(float(*i), img_box->ymin);
		i++;
	}
	resize_box(img_box, g_width, g_height);
	int x_offset = (int)(((float)g_width - (img_box->xmax - img_box->xmin))/2.0);
	int y_offset = (int)(((float)g_height - (img_box->ymax - img_box->ymin))/2.0);
	transform_box(img_box, x_offset, y_offset);
	
/*
	for( auto i = shapes[0].mesh.positions.begin(); i != shapes[0].mesh.positions.end(); i++) {
		cout << "Vertex: " << *i;
		i++;
		cout << ", " << *i;
		i++;
		cout << ", " << *i << endl;
	}
*/
	bounding_box *box = new bounding_box();
	coord *bary = new coord();
	unsigned short *zbuf = new unsigned short[g_width*g_height];
	for( auto i = shapes[0].mesh.indices.begin(); i != shapes[0].mesh.indices.end(); i++) {
		unsigned int v1 = *i;
		i++;
		unsigned int v2 = *i;
		i++;
		unsigned int v3 = *i;
		
		float x1 = shapes[0].mesh.positions.at((v1*3) + 0);
		float y1 = shapes[0].mesh.positions.at((v1*3) + 1);
		float z1 = shapes[0].mesh.positions.at((v1*3) + 2);
		float x2 = shapes[0].mesh.positions.at((v2*3) + 0);
		float y2 = shapes[0].mesh.positions.at((v2*3) + 1);
		float z2 = shapes[0].mesh.positions.at((v2*3) + 2);
		float x3 = shapes[0].mesh.positions.at((v3*3) + 0);
		float y3 = shapes[0].mesh.positions.at((v3*3) + 1);
		float z3 = shapes[0].mesh.positions.at((v3*3) + 2);
		create_box(box, x1, y1, z1, x2, y2, z2, x3, y3, z3);
		transform_box(box, 1, 1);
		if(img_box->top) {
			resize_box(box, (0.5 * (img_box->ymax - img_box->ymin)));
		}
		else {
			resize_box(box, (0.5 * (img_box->xmax - img_box->xmin)));
		}
		transform_box(box, x_offset, y_offset);
		write_coordinates(*box, img_box, image, bary, zbuf);
	}
	delete [] zbuf;
	//write out the image
	image->writeToFile(imgName);

	return 0;
}

