## CubeDynamics

This is a fluid dynamics simulator that accounts for interesting features like  surface tension.

You can watch a [youtube video](http://www.youtube.com/watch?v=RqMoEc-gecs) of its progress by clicking on the image.

[![IMAGE ALT TEXT](http://img.youtube.com/vi/RqMoEc-gecs/0.jpg)](http://www.youtube.com/watch?v=RqMoEc-gecs "Gif animation")

### Formula

The system describes a cube with a fixed number of cubical cells inside it.

All cells get updated with an incredibly simple formula based on the cellular automaton idea of computation (i.e. each cell in one state is calculated based only on the value of the 6 states bordering its sides).

To simulate the physical properties of a liquid, 4 aspects are carefully simulated for each cell and each time step.

* Conservation of momentum
* Pressure created when too much liquid gets in a single cell.
* Attraction between liquid in nearby cells.
* Gravity
* Solid container

#### Details of formula

Each cell stores the scalar mass that is inside it, and the velocity that mass is going at. A proportion of the mass in a cell is transferred to surrounding cells based on the direction and magnitude of the velocity vector. Momentum is conserved when mass is transferred. Note that there is an implicit speed of light: quantities can only travel one cell per update.

To simulate pressure, additional mass is transferred based on a pressure vector which points out in all directions equally. This pressure vector is added to the velocity vector to get actual mass transfers amounts to the cells around itself.

To simulate gravity, there is a constant uniform acceleration downward on all cells.

To simulate the sort of chemical attraction that results in surface tension, there is a force applied that pulls together nearby liquid in cells. The formula is `force = C * cuberoot(mass1) * cuberoot(mass2)`, where `C` is a hand picked constant.

When a cell tries to transfer mass into the border of the area, the mass is reflected back into itself, creating a realistic idea of a frictionless wall.


### Optimizations and performance

To improve performance, I introduced 3 main optimizations:

1. Moving main cell update computation to GPU to improve main performance.
2. Making a graphics pipeline onto 3 separate threads to create a lag free user experience, and ensure that the cell computation is running as much as possible.
3. Eliminating execution in empty areas of the space.

#### Optimization details

First and most important, I implemented the main cell update procedure in OpenCL, and directed OpenCL to use my laptop's Intel integrated graphics card. Even with my crappy integrated graphics hardware, OpenCL improved performance by around 50x over my pure C++ version. This was the difference between waiting hours and waiting minutes for something interesting to happen on larger maps.

Second, on larger maps a single threaded computation becomes very laggy or CPU bound due to the difficulty of converting cell values to triangles so that OpenGL can render the space. To prevent this, I made three threads which transferred data to each other:

* Cell update thread (no inputs, outputs cell data)
* Triangle pipeline (takes in copy of cell data, outputs triangle array)
* Main UI thread with infinite loop (takes in triangle array)

Third, I realized that most of the actual work on larger maps was uselessly calculating updates in empty areas of the map. To make this work faster, I introduced an optimization which uses the "speed of light" to reduce the area which the main cell update formula needs to execute.

#### Optimization result

The result of all this work is that both the videos are in near real time, around 50% faster than actual calculation (the second video is around 4.30 minutes, in reality, it took around 6.30 minutes). Considering the hardware I used: my middle end laptop with no discrete GPU, this is quite impressive in my mind.

### Compilation/Installation

Unfortunately, this code depends on OpenGL, OpenCL, GLEW and GLFW, which makes it very hard to install on any platform.

I developed it on Windows using MinGW, and included the required library binaries and headers in the repository, so in theory it should be relatively easy to compile if you have MinGW installed, but in practice, there are a number of things which could go wrong.