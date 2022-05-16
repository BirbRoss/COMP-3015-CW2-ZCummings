# COMP-3015-CW2-ZCummings
##Versions/Specs

- Visual Studio 2019 version 16.11.9
- .NET Framework version 4.8.04161
- Windows 11
- Nvidia GPU
- OpenGL 4.6

## How does it work?

It uses the PBR model to render a scene created using the template code to load the models.

The metalic boolean decides if a material is metalic or not, at the cost of diffuse lighting on the metalic mode. Roughness will still work, but will cause the metal to become dark. Roughness will work normally for non-metalic materials, making them matte or shiny.

The noise texture just takes some perlain noise and uses its values to mix two colours together. There is some commented code that can be uncommented to re-enable an attempt at a wood grain noise effect, however it was visually glitchy so was disabled.

## How does it fit together?

Themetically it's still a barn yard, now with the forbidden cow of biblical legend, and a smudged barn, but code wise:

*Changes from the template files & CW1 are only present in the Scenebasic_uniform.ccp/.h and the basic_uniform.frag.*

Main.cpp will just make calls to the scenebasic_uniform which will then call on the shaders to render the models for the camera.

## What makes it special

Well it's an interesting lil tech demmo scene, it suffers some issues from lack of time managment, so I appologise, one of the pigs will not be wielding a flamethrower, perhaps in a future updated if you want to see that sort of thing

## How to run

1. Download the zip from releases
2. Unzip it
3. Double click the .exe
Make sure it is in the same folder as shader and media, as shown in the standalone folder.

If you wish to run bPhong, swap "shaderToggle" on scenebasic_uniform.cpp line 61 to false, or true for PBR which is the default

## Video

[To be uploaded]

## Credit

The hay texture is in the public domain and can be found at https://pixy.org/61655/

##New and improved fog fact

Fog that forms at a higher altitude will form a stratus cloud.
