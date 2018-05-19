IQM loader for raylib
=====================
This is a .iqm model loader for raylib.
Documentation for the iqm format can be found here:
https://github.com/lsalzman/iqm
Raylib can be found here:
https://github.com/raysan5/raylib

# directories
In the example directory you will find a simple example showing how to use the iqm loader, you might have to mess with the makefile.
iqm.c contains all the functions you need, and this implements the header at include/iqm.h.
I uncommented the #define RAYMATH_HEADER_ONLY in raymath.h because it works for me.
This is a bit of a mess, but it might be fixed in a future release

The raylib dropin directory contains some of the source code, drop it in your raylib directory and recompile
Basically I implement QuaternionRotateVector in raymath.h and the actual structs and everything that directly uses opengl in rlgl.h.
The rest of the code is in iqm.h and iqm.c.
Again, this is as messy as I could make it.

In the models directory you can find an example model and two example animations in the iqm format, aswel as in a .blend file and .png texture to go with it.

A quick overview of my structs and functions and what they do:
==============================================================

#rlgl.h code, the functions here get exposed to the dll/library:
typedef struct AnimatedMesh
{
  char name[NAMELEN];             // the name of the animatedmesh, NAMELEN is 32
  int vertexCount;                // nr of vertices
  int triangleCount;              // nr of triangles
  float *vertices;                // the base pose vertex data, each vertex has 3 floats,
                                  // these do not get drawn but serve as a base for calculating the animated positions
  float *normals;                 // base pose normals, each normal has 3 floats, same as the base vertices, does not get drawn
  float *texcoords;               // uv coordinates, 2 floats per vertex
  unsigned short *triangles;      // triangle indices, 3 unsigned shorts per triangle,
                                  // triangles are formed clockwise,
                                  // if only the backfaces are drawn, look at LoadIQM(), its in a comment there somewhere
  int *weightid;                  // the weights for each vertex, 4 per vertex but at the moment only the first one is used
  float *weightbias;              // the bias for each weight, again 4 per vertex but only the first one is used

  float *avertices;               // the animated vertices, these are drawn with rlDrawAnimatedMesh and loaded with rlLoadAnimatedMesh
  float *anormals;                // the animated normals

  unsigned int vaoId;             // vaoID's, same as raylib
  unsigned int vboId[7];          // vboID's, same as raylib
} AnimatedMesh;
typedef struct Animation
{
  int jointCount;                 // the nr of joints in an animation
                                  // joints in anims only have a placeholder name
  Joint *joints;                  // the joint arrays
  int frameCount;                 // nr of frames in this animation
  float framerate;                // framerate, unsused for now
  Pose **framepose;               // a pose for each joint in each frame, called like this framepose[frame][joint]
} Animation;
typedef struct AnimatedModel
{
  int meshCount;                  // the nr of meshes in the model
  AnimatedMesh *mesh;             // the model array
  int materialCount;              // the nr of materials
  int *meshMaterials;             // the material for each mesh, eg mesh[0] would have material[meshMaterial[0]]
  Material *materials;            // an array of materials for the meshes
  int jointCount;                 // the nr of joints in the model
  Joint *joints;                  // the joint array, these do have names
  Pose *basepose;                 // the basepose array, used for calculating the final position,rotation and scale for the animated vertices

  Matrix transform;               // the transform matrix, same as raylib
} AnimatedModel;
void rlLoadAnimatedMesh(AnimatedMesh *amesh, bool dynamic);                         // loads a mesh into the GPU memory
void rlDrawAnimatedMesh(AnimatedMesh amesh, Material material, Matrix transform);   // draws the avertices and anormals arrays on the screen
void rlUnloadAnimatedMesh(AnimatedMesh *amesh);                                     // unloads a mesh and frees all the allocated memory
void rlUpdateAnimatedMesh(AnimatedMesh *amesh);                                     // updates the avertices and anormals for the mesh
                                                                                    // this uses glBufferSubData for now, might use streaming in the future
#raymath.h code:
Vector3 QuaternionRotateVector(Vector3 v, Quaternion q);                            // rotate the vector by a quaternion


#iqm.h code:
AnimatedModel LoadIQM(const char *filename);                                        // loads an IQM model from a file, should not be called directly, use LoadAnimatedModel() instead
AnimatedModel LoadAnimatedModel(const char *filename);                              // loads an IQM model from a file and sets the transform matrix and meshMaterials to default
AnimatedModel AnimatedModelAddTexture(AnimatedModel model,const char *filename);    // adds a texture to the model
AnimatedModel SetMeshMaterial(AnimatedModel model,int meshid, int textureid);       // sets the texture for a mesh, bit messy but the best I could do for now

Animation *LoadIQMAnims(const char *filename,int *animCount);                       // loads a number of animations from an IQM file, the amount of animations is put in animcount. Returns an array

void UnLoadAModel(AnimatedModel model);                                             // unloads a model, minus the materials, this will be fixed some day
void UnLoadAnim(Animation anim);                                                    // unloads a single animation, make sure to free the whole array and then free the array itself

int SkeletonsMatch(AnimatedModel model, Animation anim);                            // returns 1 if the joints for the model and the animation match, don't play the animation if it returns 0 or you get to keep the pieces

void AnimateModel(AnimatedModel model, Animation anim, int frame);                    // updates the avertices and anormals of the model according to the requestion in the animation
void DrawAnimatedmodel(AnimatedModel model,Vector3 position,float scale,Color tint);  // draws the model at position, with scale and tint, calls DrawAnimatedModelex, same thing as raylib
void DrawAnimatedmodelEx(AnimatedModel model,Vector3 position,Vector3 rotationAxis,float rotationAngle, Vector3 scale,Color tint);    // does some weird stuff, not really sure
