#include "rlgl.h"
// everything we need to load the iqm files
#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

typedef struct iqmheader
{
    char magic[16];
    unsigned int version;
    unsigned int filesize;
    unsigned int flags;
    unsigned int num_text, ofs_text;
    unsigned int num_meshes, ofs_meshes;
    unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
    unsigned int num_triangles, ofs_triangles, ofs_adjacency;
    unsigned int num_joints, ofs_joints;
    unsigned int num_poses, ofs_poses;
    unsigned int num_anims, ofs_anims;
    unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
    unsigned int num_comment, ofs_comment;
    unsigned int num_extensions, ofs_extensions;
} iqmheader;

typedef struct iqmmesh
{
    unsigned int name;
    unsigned int material;
    unsigned int first_vertex, num_vertexes;
    unsigned int first_triangle, num_triangles;
} iqmmesh;

enum
{
    IQM_POSITION     = 0,
    IQM_TEXCOORD     = 1,
    IQM_NORMAL       = 2,
    IQM_TANGENT      = 3,
    IQM_BLENDINDEXES = 4,
    IQM_BLENDWEIGHTS = 5,
    IQM_COLOR        = 6,
    IQM_CUSTOM       = 0x10
};

enum
{
    IQM_BYTE   = 0,
    IQM_UBYTE  = 1,
    IQM_SHORT  = 2,
    IQM_USHORT = 3,
    IQM_INT    = 4,
    IQM_UINT   = 5,
    IQM_HALF   = 6,
    IQM_FLOAT  = 7,
    IQM_DOUBLE = 8,
};

typedef struct iqmtriangle
{
    unsigned int vertex[3];
} iqmtriangle;

typedef struct iqmadjacency
{
    unsigned int triangle[3];
} iqmadjacency;

typedef struct iqmjoint
{
    unsigned int name;
    int parent;
    float translate[3], rotate[4], scale[3];
} iqmjoint;

typedef struct iqmpose
{
    int parent;
    unsigned int mask;
    float channeloffset[10];
    float channelscale[10];
} iqmpose;

typedef struct iqmanim
{
    unsigned int name;
    unsigned int first_frame, num_frames;
    float framerate;
    unsigned int flags;
} iqmanim;

enum
{
    IQM_LOOP = 1<<0
};

typedef struct iqmvertexarray
{
    unsigned int type;
    unsigned int flags;
    unsigned int format;
    unsigned int size;
    unsigned int offset;
} iqmvertexarray;

typedef struct iqmbounds
{
    float bbmin[3], bbmax[3];
    float xyradius, radius;
} iqmbounds;

AnimatedModel LoadIQM(const char *filename);
AnimatedModel LoadAnimatedModel(const char *filename);
AnimatedModel AnimatedModelAddTexture(AnimatedModel model,const char *filename);
AnimatedModel SetMeshMaterial(AnimatedModel model,int meshid, int textureid);
Animation *LoadIQMAnims(const char *filename,int *animCount);
void UnLoadAModel(AnimatedModel model);
void UnLoadAnim(Animation anim);
int SkeletonsMatch(AnimatedModel model, Animation anim);
void AnimateModel(AnimatedModel model, Animation anim, int frame);
void DrawAnimatedmodel(AnimatedModel model,Vector3 position,float scale,Color tint);
void DrawAnimatedmodelEx(AnimatedModel model,Vector3 position,Vector3 rotationAxis,float rotationAngle, Vector3 scale,Color tint);
