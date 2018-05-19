#include "raylib.h"         // Declares module functions

#include <stdio.h>          // Required for: FILE, fopen(), fclose(), fscanf(), feof(), rewind(), fgets()
#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: strcmp()

#include "rlgl.h"           // raylib OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2

#include "iqm.h"            // iqm header and other structs

// animated meshes
AnimatedModel LoadIQM(const char *filename)
{
  AnimatedModel model = {0};
  FILE *pfile;
  iqmheader iqm;

  iqmmesh *imesh;
  iqmtriangle *tri;
  iqmvertexarray *va;
  int *tris;
  float *vertex;
  float *normal;
  float *text;
  char *blendi;
  unsigned char *blendw;
  iqmjoint *ijoint;

  pfile = fopen(filename,"rb");
  if(!pfile)
  {
    printf("Unable to open file %s.\n",filename);
    return model;
  }
// header
  fread(&iqm,sizeof(iqmheader),1,pfile);

  if(strncmp(iqm.magic,IQM_MAGIC,sizeof(IQM_MAGIC)))
  {
    printf("Magic Number \"%s\"does not match.\n",iqm.magic);
    fclose(pfile);
    return model;
  }
  if(iqm.version != IQM_VERSION)
  {
    printf("IQM version %i is incorrect.\n",iqm.version);
    fclose(pfile);
    return model;
  }
// meshes
  imesh = malloc(sizeof(iqmmesh)*iqm.num_meshes);
  fseek(pfile,iqm.ofs_meshes,SEEK_SET);
  fread(imesh,sizeof(iqmmesh)*iqm.num_meshes,1,pfile);

  model.meshCount = iqm.num_meshes;
  model.mesh = malloc(sizeof(AnimatedMesh)*iqm.num_meshes);
  for(int i=0;i<iqm.num_meshes;i++)
  {
    fseek(pfile,iqm.ofs_text+imesh[i].name,SEEK_SET);
    fread(model.mesh[i].name,sizeof(char)*NAMELEN,1,pfile);
    model.mesh[i].vertexCount = imesh[i].num_vertexes;
    model.mesh[i].vertices = malloc(sizeof(float)*imesh[i].num_vertexes*3);
    model.mesh[i].normals = malloc(sizeof(float)*imesh[i].num_vertexes*3);
    model.mesh[i].texcoords = malloc(sizeof(float)*imesh[i].num_vertexes*2);
    model.mesh[i].weightid = malloc(sizeof(int)*imesh[i].num_vertexes*4);
    model.mesh[i].weightbias = malloc(sizeof(float)*imesh[i].num_vertexes*4);
    model.mesh[i].triangleCount = imesh[i].num_triangles;
    model.mesh[i].triangles = malloc(sizeof(unsigned short)*imesh[i].num_triangles*3);
    model.mesh[i].avertices = malloc(sizeof(float)*imesh[i].num_vertexes*3);
    model.mesh[i].anormals = malloc(sizeof(float)*imesh[i].num_vertexes*3);
  }
// tris
  tri = malloc(sizeof(iqmtriangle)*iqm.num_triangles);
  fseek(pfile,iqm.ofs_triangles,SEEK_SET);
  fread(tri,sizeof(iqmtriangle)*iqm.num_triangles,1,pfile);

  for(int m=0;m<iqm.num_meshes;m++)
  {
    int tcounter = 0;
    for(int i=imesh[m].first_triangle;i<imesh[m].first_triangle+imesh[m].num_triangles;i++)
    {
// IQM triangles are stored counter clockwise, but raylib sets opengl to clockwise drawing, so we swap them around
        model.mesh[m].triangles[tcounter+2] = tri[i].vertex[0]-imesh[m].first_vertex;
        model.mesh[m].triangles[tcounter+1] = tri[i].vertex[1]-imesh[m].first_vertex;
        model.mesh[m].triangles[tcounter] = tri[i].vertex[2]-imesh[m].first_vertex;
        tcounter+=3;
    }
  }
// vertarrays
  va = malloc(sizeof(iqmvertexarray)*iqm.num_vertexarrays);
  fseek(pfile,iqm.ofs_vertexarrays,SEEK_SET);
  fread(va,sizeof(iqmvertexarray)*iqm.num_vertexarrays,1,pfile);

  for(int i=0;i<iqm.num_vertexarrays;i++)
  {
    switch(va[i].type)
    {
      case IQM_POSITION:
        vertex = malloc(sizeof(float)*iqm.num_vertexes*3);
        fseek(pfile,va[i].offset,SEEK_SET);
        fread(vertex,sizeof(float)*iqm.num_vertexes*3,1,pfile);
        for(int m=0;m<iqm.num_meshes;m++)
        {
          int vcounter = 0;
          for(int i=imesh[m].first_vertex*3;i<imesh[m].first_vertex*3+imesh[m].num_vertexes*3;i++)
          {
              model.mesh[m].vertices[vcounter] = vertex[i];
              vcounter++;
          }
        }
        break;
      case IQM_NORMAL:
        normal = malloc(sizeof(float)*iqm.num_vertexes*3);
        fseek(pfile,va[i].offset,SEEK_SET);
        fread(normal,sizeof(float)*iqm.num_vertexes*3,1,pfile);
        for(int m=0;m<iqm.num_meshes;m++)
        {
          int vcounter = 0;
          for(int i=imesh[m].first_vertex*3;i<imesh[m].first_vertex*3+imesh[m].num_vertexes*3;i++)
          {
              model.mesh[m].normals[vcounter] = normal[i];
              model.mesh[m].anormals[vcounter] = normal[i];
              vcounter++;
          }
        }
        break;
      case IQM_TEXCOORD:
        text = malloc(sizeof(float)*iqm.num_vertexes*2);
        fseek(pfile,va[i].offset,SEEK_SET);
        fread(text,sizeof(float)*iqm.num_vertexes*2,1,pfile);
        for(int m=0;m<iqm.num_meshes;m++)
        {
          int vcounter = 0;
          for(int i=imesh[m].first_vertex*2;i<imesh[m].first_vertex*2+imesh[m].num_vertexes*2;i++)
          {
              model.mesh[m].texcoords[vcounter] = text[i];
              vcounter++;
          }
        }
        break;
      case IQM_BLENDINDEXES:
        blendi = malloc(sizeof(char)*iqm.num_vertexes*4);
        fseek(pfile,va[i].offset,SEEK_SET);
        fread(blendi,sizeof(char)*iqm.num_vertexes*4,1,pfile);

        for(int m=0;m<iqm.num_meshes;m++)
        {
          int vcounter = 0;
          for(int i=imesh[m].first_vertex*4;i<imesh[m].first_vertex*4+imesh[m].num_vertexes*4;i++)
          {
              model.mesh[m].weightid[vcounter] = blendi[i];
              vcounter++;
          }
        }
        break;
      case IQM_BLENDWEIGHTS:
        blendw = malloc(sizeof(unsigned char)*iqm.num_vertexes*4);
        fseek(pfile,va[i].offset,SEEK_SET);
        fread(blendw,sizeof(unsigned char)*iqm.num_vertexes*4,1,pfile);

        for(int m=0;m<iqm.num_meshes;m++)
        {
          int vcounter = 0;
          for(int i=imesh[m].first_vertex*4;i<imesh[m].first_vertex*4+imesh[m].num_vertexes*4;i++)
          {
              model.mesh[m].weightbias[vcounter] = blendw[i]/255.0f;
              vcounter++;
          }
        }
        break;
    }
  }
// joints, include base poses
  ijoint = malloc(sizeof(iqmjoint)*iqm.num_joints);
  fseek(pfile,iqm.ofs_joints,SEEK_SET);
  fread(ijoint,sizeof(iqmjoint)*iqm.num_joints,1,pfile);

  model.jointCount = iqm.num_joints;
  model.joints = malloc(sizeof(Joint)*iqm.num_joints);
  model.basepose = malloc(sizeof(Pose)*iqm.num_joints);
  for(int i=0;i<iqm.num_joints;i++)
  {
// joints
    model.joints[i].parent = ijoint[i].parent;
    fseek(pfile,iqm.ofs_text+ijoint[i].name,SEEK_SET);
    fread(model.joints[i].name,sizeof(char)*NAMELEN,1,pfile);
// basepose
    model.basepose[i].t.x = ijoint[i].translate[0];
    model.basepose[i].t.y = ijoint[i].translate[1];
    model.basepose[i].t.z = ijoint[i].translate[2];

    model.basepose[i].r.x = ijoint[i].rotate[0];
    model.basepose[i].r.y = ijoint[i].rotate[1];
    model.basepose[i].r.z = ijoint[i].rotate[2];
    model.basepose[i].r.w = ijoint[i].rotate[3];

    model.basepose[i].s.x = ijoint[i].scale[0];
    model.basepose[i].s.y = ijoint[i].scale[1];
    model.basepose[i].s.z = ijoint[i].scale[2];
  }
// build base pose
  for(int i=0;i<model.jointCount;i++)
  {
    if(model.joints[i].parent >= 0)
    {
      model.basepose[i].r = QuaternionMultiply(model.basepose[model.joints[i].parent].r,model.basepose[i].r);
      model.basepose[i].t = QuaternionRotateVector(model.basepose[i].t,model.basepose[model.joints[i].parent].r);
      model.basepose[i].t = Vector3Add(model.basepose[i].t,model.basepose[model.joints[i].parent].t);
      model.basepose[i].s = Vector3MultiplyV(model.basepose[i].s,model.basepose[model.joints[i].parent].s);
    }
  }
  fclose(pfile);
  free(imesh);
  free(tri);
  free(va);
  free(vertex);
  free(normal);
  free(text);
  free(blendi);
  free(blendw);
  free(ijoint);
  return model;
}
AnimatedModel LoadAnimatedModel(const char *filename)
{
  AnimatedModel out = LoadIQM(filename);
  for(int i=0;i<out.meshCount;i++)
  {
    rlLoadAnimatedMesh(&out.mesh[i], false);
  }
  out.transform = MatrixIdentity();
  out.meshMaterials = malloc(sizeof(int) * out.meshCount);
  out.materials = NULL;
  out.materialCount = 0;
  for(int i=0;i<out.meshCount;i++)
  {
    out.meshMaterials[i] = -1;
  }
  return out;
}
AnimatedModel AnimatedModelAddTexture(AnimatedModel model,const char *filename)
{
  Texture2D texture = LoadTexture(filename);
  model.materials = realloc(model.materials,sizeof(Material) * (model.materialCount + 1));
  model.materials[model.materialCount] = LoadMaterialDefault();
  model.materials[model.materialCount].maps[MAP_DIFFUSE].texture = texture;
  model.materialCount++;
  return model;
}
AnimatedModel SetMeshMaterial(AnimatedModel model,int meshid, int textureid)
{
  if(meshid > model.meshCount)
  {
    printf("MeshId greater than meshCount\n");
    return model;
  }
  if(textureid > model.materialCount)
  {
    printf("textureid greater than materialCount\n");
    return model;
  }
  model.meshMaterials[meshid] = textureid;
  return model;
}
Animation *LoadIQMAnims(const char *filename, int *animCount)
{
  Animation *animation = NULL;
  FILE *pfile;
  iqmheader iqm;
  *animCount = 0;

  pfile = fopen(filename,"rb");
  if(!pfile)
  {
    printf("Unable to open file %s.\n",filename);
    return NULL;
  }
// header
  fread(&iqm,sizeof(iqmheader),1,pfile);

  if(strncmp(iqm.magic,IQM_MAGIC,sizeof(IQM_MAGIC)))
  {
    printf("Magic Number \"%s\"does not match.\n",iqm.magic);
    fclose(pfile);
    return NULL;
  }
  if(iqm.version != IQM_VERSION)
  {
    printf("IQM version %i is incorrect.\n",iqm.version);
    fclose(pfile);
    return NULL;
  }
// header
  animation = malloc(sizeof(Animation)*iqm.num_anims);
  *animCount = iqm.num_anims;
// joints
  iqmpose *poses;
  poses = malloc(sizeof(iqmpose)*iqm.num_poses);
  fseek(pfile,iqm.ofs_poses,SEEK_SET);
  fread(poses,sizeof(iqmpose)*iqm.num_poses,1,pfile);

  for(int i=0;i<iqm.num_anims;i++)
  {
    animation[i].jointCount = iqm.num_poses;
    animation[i].joints = malloc(sizeof(Joint)*iqm.num_poses);
    for(int j=0;j<iqm.num_poses;j++)
    {
      strcpy(animation[i].joints[j].name,ANIMJOINTNAME);
      animation[i].joints[j].parent = poses[j].parent;
    }
  }

// animations
  iqmanim *anims;
  anims = malloc(sizeof(iqmanim)*iqm.num_anims);
  fseek(pfile,iqm.ofs_anims,SEEK_SET);
  fread(anims,sizeof(iqmanim)*iqm.num_anims,1,pfile);

  for(int i=0;i<iqm.num_anims;i++)
  {
    animation[i].frameCount = anims[i].num_frames;
    animation[i].framerate = anims[i].framerate;
  }

// frameposes
  unsigned short *framedata = malloc(sizeof(unsigned short)*iqm.num_frames*iqm.num_framechannels);
  fseek(pfile,iqm.ofs_frames,SEEK_SET);
  fread(framedata,sizeof(unsigned short)*iqm.num_frames*iqm.num_framechannels,1,pfile);

  for(int i=0;i<iqm.num_anims;i++)
  {
    animation[i].framepose = malloc(sizeof(Pose*)*anims[i].num_frames);
      for(int j=0;j<anims[i].num_frames;j++)
      {
        animation[i].framepose[j] = malloc(sizeof(Pose)*iqm.num_poses);
      }
  }

  for(int a=0;a<iqm.num_anims;a++)
  {
    int dcounter = anims[a].first_frame*iqm.num_framechannels;
    for(int frame=0;frame<anims[a].num_frames;frame++)
    {
      for(int i=0;i<iqm.num_poses;i++)
      {
        animation[a].framepose[frame][i].t.x += poses[i].channeloffset[0];
        if(poses[i].mask & 0x01)
        {
          animation[a].framepose[frame][i].t.x += framedata[dcounter] * poses[i].channelscale[0];
          dcounter++;
        }
        animation[a].framepose[frame][i].t.y += poses[i].channeloffset[1];
        if(poses[i].mask & 0x02)
        {
          animation[a].framepose[frame][i].t.y += framedata[dcounter] * poses[i].channelscale[1];
          dcounter++;
        }
        animation[a].framepose[frame][i].t.z = poses[i].channeloffset[2];
        if(poses[i].mask & 0x04)
        {
          animation[a].framepose[frame][i].t.z += framedata[dcounter] * poses[i].channelscale[2];
          dcounter++;
        }
        animation[a].framepose[frame][i].r.x += poses[i].channeloffset[3];
        if(poses[i].mask & 0x08)
        {
          animation[a].framepose[frame][i].r.x += framedata[dcounter] * poses[i].channelscale[3];
          dcounter++;
        }
        animation[a].framepose[frame][i].r.y += poses[i].channeloffset[4];
        if(poses[i].mask & 0x10)
        {
          animation[a].framepose[frame][i].r.y += framedata[dcounter] * poses[i].channelscale[4];
          dcounter++;
        }
        animation[a].framepose[frame][i].r.z = poses[i].channeloffset[5];
        if(poses[i].mask & 0x20)
        {
          animation[a].framepose[frame][i].r.z += framedata[dcounter] * poses[i].channelscale[5];
          dcounter++;
        }
        animation[a].framepose[frame][i].r.w = poses[i].channeloffset[6];
        if(poses[i].mask & 0x40)
        {
          animation[a].framepose[frame][i].r.w += framedata[dcounter] * poses[i].channelscale[6];
          dcounter++;
        }
        animation[a].framepose[frame][i].s.x = poses[i].channeloffset[7];
        if(poses[i].mask & 0x80)
        {
          animation[a].framepose[frame][i].s.x += framedata[dcounter] * poses[i].channelscale[7];
          dcounter++;
        }
        animation[a].framepose[frame][i].s.y = poses[i].channeloffset[8];
        if(poses[i].mask & 0x100)
        {
          animation[a].framepose[frame][i].s.y += framedata[dcounter] * poses[i].channelscale[8];
          dcounter++;
        }
        animation[a].framepose[frame][i].s.z = poses[i].channeloffset[9];
        if(poses[i].mask & 0x200)
        {
          animation[a].framepose[frame][i].s.z += framedata[dcounter] * poses[i].channelscale[9];
          dcounter++;
        }
        animation[a].framepose[frame][i].r = QuaternionNormalize(animation[a].framepose[frame][i].r);
      }
    }
  }
// build frameposes
  for(int j=0;j<iqm.num_anims;j++)
  {
    for(int frame=0;frame<anims[j].num_frames;frame++)
    {
      for(int i=0;i<animation[j].jointCount;i++)
      {
        if(animation[j].joints[i].parent >= 0)
        {
          animation[j].framepose[frame][i].r = QuaternionMultiply(animation[j].framepose[frame][animation[j].joints[i].parent].r,animation[j].framepose[frame][i].r);
          animation[j].framepose[frame][i].t = QuaternionRotateVector(animation[j].framepose[frame][i].t,animation[j].framepose[frame][animation[j].joints[i].parent].r);
          animation[j].framepose[frame][i].t = Vector3Add(animation[j].framepose[frame][i].t,animation[j].framepose[frame][animation[j].joints[i].parent].t);
          animation[j].framepose[frame][i].s = Vector3MultiplyV(animation[j].framepose[frame][i].s,animation[j].framepose[frame][animation[j].joints[i].parent].s);
        }
      }
    }
  }
  free(framedata);
  free(anims);
  free(poses);
  fclose(pfile);
  return animation;
}
int SkeletonsMatch(AnimatedModel model, Animation anim)
{
  if(model.jointCount != anim.jointCount)
    return 0;
  for(int i=0;i<model.jointCount;i++)
  {
    if(model.joints[i].parent != anim.joints[i].parent)
      return 0;
  }
  return 1;
}
void UnLoadAModel(AnimatedModel model)
{
  free(model.materials);
  free(model.joints);
  free(model.basepose);
  for(int i=0;i<model.meshCount;i++)
  {
    rlUnloadAnimatedMesh(&model.mesh[i]);
  }
  free(model.mesh);
}
void UnLoadAnim(Animation anim)
{
  free(anim.joints);
  for(int i=0;i<anim.frameCount;i++)
    free(anim.framepose[i]);
    printf("dun");
  free(anim.framepose);
}
void DrawAnimatedmodel(AnimatedModel model,Vector3 position,float scale,Color tint)
{
  Vector3 vScale = { scale, scale, scale };
  Vector3 rotationAxis = { 0.0f,0.0f,0.0f };
  DrawAnimatedmodelEx(model,position,rotationAxis,0.0f, vScale,tint);
}
void DrawAnimatedmodelEx(AnimatedModel model,Vector3 position,Vector3 rotationAxis,float rotationAngle, Vector3 scale,Color tint)
{
  if(model.materialCount == 0)
  {
    printf("No materials set, can't draw animated mesh\n");
    return;
  }
  Matrix matScale = MatrixScale(scale.x,scale.y,scale.z);
  Matrix matRotation = MatrixRotate(rotationAxis,rotationAngle*DEG2RAD);
  Matrix matTranslation = MatrixTranslate(position.x,position.y,position.z);

  Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale,matRotation),matTranslation);
  model.transform = MatrixMultiply(model.transform,matTransform);
  for(int i=0;i<model.meshCount;i++)
  {
    rlUpdateAnimatedMesh(&model.mesh[i]);
    rlDrawAnimatedMesh(model.mesh[i],model.materials[model.meshMaterials[i]],MatrixIdentity());
  }
}
void AnimateModel(AnimatedModel model, Animation anim, int frame)
{
  if(frame >= anim.frameCount)
    frame = frame%anim.frameCount;
  for(int m=0;m<model.meshCount;m++)
  {
    Vector3 outv = {0};
    Vector3 outn = {0};

    Vector3 baset = {0};
    Quaternion baser = {0};
    Vector3 bases = {0};

    Vector3 outt = {0};
    Quaternion outr = {0};
    Vector3 outs = {0};

    int vcounter = 0;
    int wcounter = 0;
    int weightid = 0;
    for(int i=0;i<model.mesh[m].vertexCount;i++)
    {
      weightid = model.mesh[m].weightid[wcounter];
      baset = model.basepose[weightid].t;
      baser = model.basepose[weightid].r;
      bases = model.basepose[weightid].s;
      outt = anim.framepose[frame][weightid].t;
      outr = anim.framepose[frame][weightid].r;
      outs = anim.framepose[frame][weightid].s;
// vertices
      outv = (Vector3){model.mesh[m].vertices[vcounter],model.mesh[m].vertices[vcounter+1],model.mesh[m].vertices[vcounter+2]};
      outv = Vector3MultiplyV(outv,outs);
      outv = Vector3Subtract(outv,baset);
      outv = QuaternionRotateVector(outv,QuaternionMultiply(outr,QuaternionInvert(baser)));
      outv = Vector3Add(outv,outt);
      model.mesh[m].avertices[vcounter] = outv.x;
      model.mesh[m].avertices[vcounter+1] = outv.y;
      model.mesh[m].avertices[vcounter+2] = outv.z;
// normals
      outn = (Vector3){model.mesh[m].normals[vcounter],model.mesh[m].normals[vcounter+1],model.mesh[m].normals[vcounter+2]};
      outn = QuaternionRotateVector(outn,QuaternionMultiply(outr,QuaternionInvert(baser)));
      model.mesh[m].anormals[vcounter] = outn.x;
      model.mesh[m].anormals[vcounter+1] = outn.y;
      model.mesh[m].anormals[vcounter+2] = outn.z;
      vcounter+=3;
      wcounter+=4;
    }
  }
}
