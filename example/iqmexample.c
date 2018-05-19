#include "iqmexample.h"

int main()
{
// basic raylib setup
  int screenWidth = 800;
  int screenHeight = 800;
  InitWindow(screenWidth,screenHeight,"IQM");
// load the model, this needs to be behind the InitWindow call or it breaks
  AnimatedModel model;
  model = LoadAnimatedModel("../models/guy.iqm");
// and give it a texture, theres only 1 mesh and now 1 material (both at index 0), thats what the 2 0's are
  model = AnimatedModelAddTexture(model,"../models/guytex.png");
  model = SetMeshMaterial(model,0,0);
// and the animations
  Animation *anims;
  int animCount;
  anims = LoadIQMAnims("../models/guyanim.iqm",&animCount);
// raylib camera and some more setup
  Camera camera;
  camera.position = (Vector3){10.0f,10.0f,10.0f};
  camera.target = (Vector3){0.0f,0.0f,0.0f};
  camera.up = (Vector3){0.0f,1.0f,0.0f};
  camera.fovy = 45.0f;
  camera.type = CAMERA_PERSPECTIVE;
  SetCameraMode(camera, CAMERA_FREE);
  SetTargetFPS(60);
  int frame = 0;
  int animcnt = 0;
  while (!WindowShouldClose())
  {
// animation plays when spacebar is held down
    if(IsKeyDown(KEY_SPACE))
      frame++;
// animation changed when the A key is released
    if(IsKeyReleased(KEY_A))
    {
      animcnt++;
      if(animcnt > 1)
        animcnt = 0;
    }
// more raylib
    ClearBackground(RAYWHITE);
    UpdateCamera(&camera);
    BeginDrawing();
      BeginMode3D(camera);
// animate the model with animation animcnt at frame
        AnimateModel(model,anims[animcnt],frame);
// and draw it at position 0,0,0 with scale 1 and a tint, although the tint does not get used just yet
        DrawAnimatedmodel(model,(Vector3){0.0f,0.0f,0.0f},1.0f,WHITE);
// again raylib
      EndMode3D();
    EndDrawing();
  }
  CloseWindow();
// unload the model and its meshes
  UnLoadAModel(model);
// unload the animations, aswel as the actual animation array
  UnLoadAnim(*anims);
  free(anims);
  return 0;
}
