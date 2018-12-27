This is a fluid dynamics simulator that accounts for things like surface tension.

You can watch the youtube video of its progress by clicking on the image.

[![IMAGE ALT TEXT](http://img.youtube.com/vi/fHrjmD4jiyo/0.jpg)](http://www.youtube.com/watch?v=fHrjmD4jiyo "Gif animation")

The map size can change. A larger map is slower, but leads to more interesting interactions due to the increased complexity.

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

To simulate pressure, additional mass is transferred based on a pressure vector which is added to the

To simulate gravity, there is a constant uniform acceleration downward on all cells.

To simulate the sort of chemical attraction that results in surface tension, there is a force applied that pulls together nearby liquid in cells. The formula is `force = cuberoot(mass1) * cuberoot(mass1)`

When a cell tries to transfer mass into the border of the area, the mass is reflected back into itself, creating a realistic idea of a frictionless wall.


### Optimizations and performance

To make this procedure faster, I implemented the main cell update procedure in OpenCL, and introduced some optimizations to allow it to not calculate results in cells with no mass anywhere around them (and, the speed of light is low, so this works very well).

The result is that both the videos are in near real time, around 50% faster than actual calculation.  

### Install
