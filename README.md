# tool

## A collection of helper utility tools
The tools is optimized for fast compilation.

Currently the library provides a command line tool, which auto-generates description for assets.

Supported assets:
- Images
- Texture Atlasses
- Fonts
- Sounds

The assets:
- are being auto-generated frrom the command line tool called "resource_builder" using a set of description files.
- can easily be imported within a project using the library resource utils
- The library can be found here: https://github.com/zhivkopetrov/resource_utils

## Usage
The tool:
- relies on description file (ending in *.rsrc extension)
Example: my_awesome_game.rsrc
- can combine multiple *.rsrc input file into single output file
- auto-generates C++ headers files with the names of the described assets
- auto-generates binary files with asses description
- takes a list of paramets - folders to the filesystem and recursively scans for *.rsrc files

Example usage using out-of-source build in 'build' folder
```
root_folder
   |--build
   |--dev_battle_gui
         |--resources
              |--DevBattleGui.rsrc
```

From 'build' directory:
```
./tools/resource_builder/resource_builder dev_battle_gui
```

## .rsrc file description
The file should follow the following syntax

```
tag=[<asset name/ID here>]
type=<tag type> #image, sprite, sprite_manual, font, sound 
path=<relative path to file from .rsrc file>
description=<additional description based on the asset type>
position=<initial position on screen where 0,0 is top left on the scrreen>
load=<load the asset on system startup or just load it's definition and let it be loaded at runtime> #on_init, on_demannd
```

### Image example
```
tag=[TILE_SURFACE]
type=image
path=images/map.png
description=empty #field not used on images
position=0,0
load=on_init
```

### Sprite example
```
tag=[TILE_SURFACE]
type=sprite
path=images/tile_surface.jpg
description=78,64,4,0 #[frameWidth, frameHeight, numberOfFrames, frameSpacing]
position=0,0
load=on_demand
```

Note: 
The description param automatically produces equal destination frames from the provided source image.
It also, automatically detects horizontal, vertical or mixed sprite layout.
FrameSpacing (if any) is applied based on the source layout.
The descrition population will fail if bigger dimensions that what the source image provides are given.

### Sprite Manual example
```
tag=[PARTIAL_SURFACE]
type=sprite_manual
path=images/tile_surface.jpg
description=78,64,40,22 #frame0 details - [sourceX, sourceY, width, height]
description=200,23,120,42 #frame1 details - [sourceX, sourceY, width, height]
description=372,264,47,163 #frameN details - [sourceX, sourceY, width, height]
position=0,0
load=on_demand
```

Note: 
Any number of description can be used on this mode.
This is perfect for texture atlasses, where assets from different sources are combined in one big texture.

### Font example
```
tag=[MY_AWESOME_FONT]
type=font
path=fonts/my_awesome_font.ttf
description=15 #font size
```

Note: 
Fonts are always loaded on initialiazition.

### Sound

Two sound types are supported: 'chunk' and 'music'
The music files will be streamed, while chunks are fully loaded.
Four sound volume levels are supported: 'low', 'medium', 'high', 'very_high'
Sounds are always loaded on initialiazition.

### Sound example

```
tag=[MY_AWESOME_SOUNDTRACK]
type=sound
path=fonts/my_awesome_soundtrack.ogg
description=music, high

tag=[MY_AWESOME_ALERT_SOUND]
type=sound
path=fonts/beep.ogg
description=chunk, medium
```

## Usage from plain CMake or ROS1(catkin) / ROS2(colcon) meta-build systems

- clone the repository in your file system
- Example usage projects: 
https://github.com/zhivkopetrov/dev_battle.git
https://github.com/zhivkopetrov/robotics_v1


## Dependencies
- cmake_helpers - https://github.com/zhivkopetrov/cmake_helpers.git
- utils - https://github.com/zhivkopetrov/utils
- resource_utils - https://github.com/zhivkopetrov/resource_utils

## Supported Platforms
Linux:
  - g++ (9.3 and above)
    - Tested up to g++ 12.1
  - clang++ (10 and above)
    - Tested up to clang++ 14.0
  - Emscripten (3.1.28 and above)
    - emcc/em++
  - Robot Operating System 2 (ROS2)
    - Through colcon meta-build system (CMake based)
  - Robot Operating System 1 (ROS1)
    - Through catkin meta-build system (CMake based)
      - Due to soon ROS1 end-of-life catkin builds are not actively supported

Windows:
  - MSVC++ (14.20 and above) Visual Studio 2019
    - Tested up to 17.30 Visual Studio 2022
  - Emscripten (3.1.28 and above)
    - emcc/em++
    - NOTE: non-MSVC CMake build generator is needed
      - For example: ninja
  - ROS1/ROS2
    - Although the code is ROS compatible, actual ROS functionalities have not been tested on Windows