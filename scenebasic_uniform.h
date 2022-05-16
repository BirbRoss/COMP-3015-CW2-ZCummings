#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include "helper/glslprogram.h"

//Helper meshs
#include "helper/objmesh.h"
#include "helper/plane.h"
#include "helper/cube.h"

#include "helper/texture.h"
#include "helper/noisetex.h"



class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;

    //My meshes
    Plane plane;
    Plane animatedPlane;
    std::unique_ptr<ObjMesh> pig;
    std::unique_ptr<ObjMesh> cow;
    std::unique_ptr<ObjMesh> barn;
    std::unique_ptr<ObjMesh> trough;
    Cube cube;

    void compile();
    void setMatrices();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
