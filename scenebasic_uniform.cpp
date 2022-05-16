#include "scenebasic_uniform.h"

#include <sstream>
#include <iostream>

using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::mat4;

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include "helper/glutils.h"

SceneBasic_Uniform::SceneBasic_Uniform() : plane(30.0f, 30.0f, 100, 100), animatedPlane(2.0f, 1.0f, 100, 100)
{
    pig = ObjMesh::load("media/pig_triangulated.obj",true);
    cow = ObjMesh::load("media/spot_triangulated.obj", true);
    barn = ObjMesh::load("media/simple_barn.obj", true);
    trough = ObjMesh::load("media/water_trough.obj", true);
    
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);

    projection = mat4(1.0f);
    float x, z;

    //Sets light positions, spacing them evenly.
    for (int i = 0; i < 3; i++)
    {
        std::stringstream name;
        name << "lights[" << i << "].lPosition";
        x = 2.0f * cosf((glm::two_pi<float>() / 3) * i);
        z = 2.0f * sinf((glm::two_pi<float>() / 3) * i);
        prog.setUniform(name.str().c_str(), view * glm::vec4(x, 1.2f, z + 5.0f, 1.0f));
    }


    //Controls light colours, and intensity if it's the same value like 1.0/1.0/1.0
    prog.setUniform("lights[0].L", vec3(5.0f));
    prog.setUniform("lights[1].L", vec3(5.0f));
    prog.setUniform("lights[2].L", vec3(5.0f));

    //Controls the fog distance and colour
    prog.setUniform("Fog.maxDist", 20.0f);
    prog.setUniform("Fog.minDist", 7.5f);
    prog.setUniform("Fog.fColour", vec3(0.67f, 0.84f, 0.9f));

    //CHANGE THIS TO SWAP BETWEEN PBR AND BPHONG!
    prog.setUniform("shaderToggle", true);

    //Loads texture from file into buffers
    GLuint brick = Texture::loadTexture("media/texture/brick1.jpg");
    GLuint hay = Texture::loadTexture("media/texture/hay.png");


    /*Establishes the virtual log to take the noise texture from || Removed do to visual glitchiness, which is documented in the video
    mat4 slice;
    slice = glm::rotate(slice, glm::radians(90.0f), vec3(1.0, 0.0, 0.0));
    slice = glm::rotate(slice, glm::radians(90.0f), vec3(0.0, 0.0, 1.0));
    slice = glm::scale(slice, vec3(5.0, 5.0, 1.0));
    slice = glm::translate(slice, vec3(0.0, 0.1, 0.0));
    prog.setUniform("Material.Slice", slice);*/

    //Generates perlin noise texture
    GLuint noiseTex = NoiseTex::generate2DTex(6.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brick);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, hay);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
    //Creates angles for smooth spin around 0,0 0,0 0,0
    const float radius = 3.0f;
    float camX = sin(t) * radius;
    float camZ = cos(t) * radius;

    //Camera position (relative to pig | X/Y/Z) | Where camera is pointed at | The top vector of the camera
    view = glm::lookAt(vec3(camX, 1.25f, camZ), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
}

void SceneBasic_Uniform::render()
{
    //Clears buffers and sets background colour to light/sky blue
    glClearColor(0.67f, 0.84f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    

    //Rough pig render | .7/.4/.4 = Pink
    //Colour becomes the metalic albedo when metal != 0
    prog.setUniform("Material.Rough", 0.9f);
    prog.setUniform("Material.Metal", 0);
    prog.setUniform("Material.Colour", 0.7f, 0.4f, 0.4f);
    prog.setUniform("Material.applyBrickHayTex", false);
    prog.setUniform("Material.applyNoiseTex", false);

    prog.setUniform("Material.Kd", 0.7f, 0.4f, 0.4f);
    prog.setUniform("Material.Ks", 0.7f, 0.4f, 0.4f);
    prog.setUniform("Material.Ka", 0.7f, 0.4f, 0.4f);
    prog.setUniform("Material.Shininess", 180.0f);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    setMatrices();
    pig->render();

    //Smooth pig render
    //Colour becomes the metalic albedo when metal != 0
    prog.setUniform("Material.Rough", 0.1f);
    prog.setUniform("Material.Metal", 0);
    prog.setUniform("Material.Colour", 0.7f, 0.4f, 0.4f);
    prog.setUniform("Material.applyBrickHayTex", false);
    prog.setUniform("Material.applyNoiseTex", false);
    //bPhong uniforms
    prog.setUniform("Material.Kd", 0.7f, 0.4f, 0.4f);
    prog.setUniform("Material.Ks", 0.7f, 0.4f, 0.4f);
    prog.setUniform("Material.Ka", 0.7f, 0.4f, 0.4f);
    prog.setUniform("Material.Shininess", 180.0f);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(30.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(1.0f, 0.0f, -0.5f));
    setMatrices();
    pig->render();

    //Cow render | 1/1/1 = white
    prog.setUniform("Material.Rough", 0.45f);
    prog.setUniform("Material.Metal", 1);
    prog.setUniform("Material.Colour", 1.0f, 0.86f, 0.67f);
    prog.setUniform("Material.applyBrickHayTex", false);
    prog.setUniform("Material.applyNoiseTex", false);

    prog.setUniform("Material.Kd", 1.0f, 1.0f, 1.0f);
    prog.setUniform("Material.Ks", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 100.0f);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(30.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(1.0f, 0.45f, 1.5f));
    setMatrices();
    cow->render();

    //Barn render | 1/1/1 = white
    prog.setUniform("Material.Rough", 0.9f);
    prog.setUniform("Material.Metal", 0);
    //prog.setUniform("Material.Colour", 0.3f, 0.15f, 0.0f);
    prog.setUniform("Material.applyBrickHayTex", false);
    prog.setUniform("Material.applyNoiseTex", true);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(30.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(1.0f, 1.0f, 1.75f));
    model = glm::scale(model, vec3(0.5f));
    setMatrices();
    barn->render();

    //Barn render | 1/1/1 = white
    prog.setUniform("Material.Rough", 0.9f);
    prog.setUniform("Material.Metal", 1);
    prog.setUniform("Material.Colour", 0.3f, 0.15f, 0.0f);
    prog.setUniform("Material.applyBrickHayTex", false);
    prog.setUniform("Material.applyNoiseTex", true);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(2.0f, -0.25f, 2.0f));
    model = glm::scale(model, vec3(0.45f));
    setMatrices();
    trough->render();

    //Plane render | .1/.3/.1 = Green
    prog.setUniform("Material.Rough", 0.1f);
    prog.setUniform("Material.Metal", 0);
    prog.setUniform("Material.Colour", 0.1f, 0.3f, 0.1f);
    prog.setUniform("Material.applyBrickHayTex", false);
    prog.setUniform("Material.applyNoiseTex", false);

    prog.setUniform("Material.Kd", 0.1f, 0.3f, 0.1f);
    prog.setUniform("Material.Ks", 0.1f, 0.3f, 0.1f);
    prog.setUniform("Material.Ka", 0.1f, 0.3f, 0.1f);
    prog.setUniform("Material.Shininess", 150.0f);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, -0.45f, 0.0f));
    setMatrices();
    plane.render();

    //Animated plane render | .1/.3/.5 = Blue
    prog.setUniform("Material.Rough", 0.1f);
    prog.setUniform("Material.Metal", 0);
    prog.setUniform("Material.Colour", 0.1f, 0.3f, 0.5f);
    prog.setUniform("Material.applyBrickHayTex", false);
    prog.setUniform("Material.applyNoiseTex", false);

    prog.setUniform("Material.Kd", 0.1f, 0.3f, 0.5f);
    prog.setUniform("Material.Ks", 0.1f, 0.3f, 0.5f);
    prog.setUniform("Material.Ka", 0.1f, 0.3f, 0.5f);
    prog.setUniform("Material.Shininess", 150.0f);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(2.0f, 0.0f, -2.0f));
    model = glm::scale(model, vec3(0.055f, 0.025f, 0.025f));
    setMatrices();
    plane.render();

    //Totally a food trough render | .6/.2/.1 = Brick red
    prog.setUniform("Material.Rough", 0.9f);
    prog.setUniform("Material.Metal", 0);
    prog.setUniform("Material.Colour", 0.6f, 0.2f, 0.1f);
    prog.setUniform("Material.applyBrickHayTex", true);
    prog.setUniform("Material.applyNoiseTex", false);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(1.0f, -0.55f, 0.0f));
    setMatrices();
    cube.render();
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    
    projection = glm::perspective(glm::radians(70.0f), (float)w/h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::setMatrices()
{
    mat4 mv = view * model;

    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]),vec3(mv[1]), vec3(mv[2])));
    prog.setUniform("MVP", projection * mv);

}