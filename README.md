# Icosidodecahedron Vector Art

## Project Description

The inspiration for this project comes from some previous work (in reality, just some time spent messing around in Blender in early high school) with a 3D icosahedron fractal created by iteratively copying and translating regular icosahedra to form the vertices of the next stage cell. While [this site](http://www.fractalnature.com/icosahedronfractal.html) was not used for inspiration, it shows off the same fractal that inspired this project.

I wanted to create vector art of this fractal and of a similar fractal constructed of [icosidodecahedra](https://en.wikipedia.org/wiki/Icosidodecahedron) (which are, coincidentally, my favorite polyhedra) from a variety of perspectives. The idea behind vector art instead of a raster image is that vector art can be zoomed-in losslessly, since the image is just a definition of points, lines, curves, and shapes in 2D space, which pairs well with fractals.

The first stage to this was finding a way to define these fractals. Due to what some might label a "floating point aversion", I wanted to do this using a definition of the coordinates where each component was a sum of integer multiples of powers of φ, the golden ratio, which is doing most of the heavy lifting in any situation where there's a lot of "five-y" geometry. Was this necessary in terms of providing a more accurate end result than just manipulating floats the whole way? No, probably not. Did this help me sleep easier at night? Also, no, probably not, since this launched me down the rabbit hole of trying to find an efficient algorithm of reducing one of these φ polynomials (termed "phi vectors", or `CPhiVector` in code) into its "simplest configuration" (which was never something I was able to boil down to a succinct definition that I was satisfied with). That said, I was more content with my work knowing that all the floating point arithmetic was just computed once at the end, right before the optional rotation in 3D space to generate the 3-fold or 5-fold symmetry orientations. As one might expect, I just chose to define edges in terms of 2 vertex IDs, and faces in terms of 3 or 5.

At this point, I was able to generate some basic `.svg`s of these fractals, but I wanted to spice things up by adding color, based on essentially the highest fractal iteration on which any given vertex, edge, or face sat (see "Icosahedron, 3-Fold Symmetry, 2" in the gallery for a good illustration of this). This took a little bit of time, but not too much in the grand scope of the project, and it produced interesting results. The effect that I was expecting wasn't really captured, since at the end of the day, the vast majority of vertices, edges, and faces lie on the smallest-iteration polyhedral cells, so most of the image just looks one solid color.

I found it difficult, however, to observe the larger order fractals due to how large the files were. I'm fairly certain that at one point I waited 40+ minutes one night for an iteration 3 (starting at iteration 0) icosidodecahedron image to load in, and that was with fully opaque faces. It didn't take long to realize that a lot of the time spent loading in these images (and a lot of the image data itself) was for vertices, edges, and faces that were completely hidden from the final product due to being overlapped by faces above them. This is where the project (and more importantly, project scope and duration) got interesting.

I decided that I wanted to improve the program by excluding faces (and their edges/vertices if those weren't associated with non-excluded faces) that were occluded by faces closer to the "viewing plane" (the fractals are captured from an orthogonal projection, not a perspective one). Described in a different way, imagine there's a planar light source shining directly downward in the *z*-axis (*z* is up, *x* is to the right, and *y* is forward—I will die on this hill), and this is positioned above the end-result fractal. If any face is covered completely in the collective shadow formed by the faces above it, that face can be excluded from the `.svg`. To do this, I needed to write an algorithm to 1: convert a 3D face into a 2D polygon (basically project it into the *xy* plane) and 2: compute the union of 2 "polygons" (explanation for the quotations provided later). Part 1 was easy: just take the *x* and *y* components of a vector. Part 2 was a little more complicated, and honestly where the majority of time spent on this project went. This was difficult, since this "polygon" resulting from the union operations was really a complex mask that needed to be able to do a variety of things, including, but not limited to:
* represent disjoint shapes (think the two dots of a colon, ':')
* represent a shape with negative space in it, like an 'o'
* represent nestings of the above two situations
* as well as properly join together shapes that are "kissing" or "flush"

I'm not going to walk through the full algorithm that I ended up settling with, but I can say that it involved a lot of convoluted logic. As an example of this, here's a brief overview of a concept I deemed "ghost loops":

  Consider 2 concentric circular rings, one with outer radius 3*r* and inner radius 2*r*, and the other with outer radius 2*r* and inner radius *r*. The inner radius of the larger ring and the outer radius of the smaller ring are the same circle, and so this won't be present on the union of these two rings, but when progressing from the the outside inwards, that removed loop, or "ghost loop" still "closes" the outer and "opens" the inner shape, where "closed" and "open" just refer to whether, in any point of 2D space, a given shape/polygon is contributing to the resultant union formed by that shape with another shape.

It took many iterations on this algorithm to get to a point where it passed all the different test cases I could throw its way (you can run the test cases yourself by `TestPolygonUnion()` function in `main()` in `main.cpp`, which is currently commented out).

With confidence that this algorithm was now working correctly (though implementing a concept of face normals could've cut out 1/2 of the faces tested with relative ease), I was ready to give it a go. My favorite orientation for these fractals is that which produces 5-fold rotational symmetry, so I only ever ran the iteration 3 fractal generation from this perspective, since this itself took about a 36 hours running on my laptop. 

## Gallery

### Icosahedron

#### Axis-Orthogonal

| **Iteration 2** | **Iteration 3** |
| :-: | :-: |
| ![Iteration 2 of the icosahedron fractal, viewed from the axis-orthogonal perspective](/images/png/Icosahedron_AxisOrthogonal_2.png) | ![Iteration 3 of the icosahedron fractal, viewed from the axis-orthogonal perspective](/images/png/Icosahedron_AxisOrthogonal_3.png) |

#### 3-Fold Symmetry

| **Iteration 2** | **Iteration 3** |
| :-: | :-: |
| ![Iteration 2 of the icosahedron fractal, viewed from the perspective with 3-fold rotational symmetry](/images/png/Icosahedron_3FoldSymmetry_2.png) | ![Iteration 3 of the icosahedron fractal, viewed from the perspective with 3-fold rotational symmetry](/images/png/Icosahedron_3FoldSymmetry_3.png) |

#### 5-Fold Symmetry

| **Iteration 2** | **Iteration 3** |
| :-: | :-: |
| ![Iteration 2 of the icosahedron fractal, viewed from the perspective with 5-fold rotational symmetry](/images/png/Icosahedron_5FoldSymmetry_2.png) | ![Iteration 3 of the icosahedron fractal, viewed from the perspective with 5-fold rotational symmetry](/images/png/Icosahedron_5FoldSymmetry_3.png) |

### Icosidodecahedron

#### Axis-Orthogonal

| **Iteration 2** |
| :-: |
| ![Iteration 2 of the icosidodecahedron fractal, viewed from the axis-orthogonal perspective](/images/png/Icosidodecahedron_AxisOrthogonal_2.png) |

#### 3-Fold Symmetry

| **Iteration 2** |
| :-: |
| ![Iteration 2 of the icosidodecahedron fractal, viewed from the perspective with 3-fold rotational symmetry](/images/png/Icosidodecahedron_3FoldSymmetry_2.png) |

#### 5-Fold Symmetry

| **Iteration 2** | **Iteration 3** |
| :-: | :-: |
| ![Iteration 2 of the icosidodecahedron fractal, viewed from the perspective with 5-fold rotational symmetry](/images/png/Icosidodecahedron_5FoldSymmetry_2.png) | ![Iteration 3 of the icosidodecahedron fractal, viewed from the perspective with 5-fold rotational symmetry. It's mostly cyan with a large quasi-circular hole in the center. Various other holes can be seen, forming pentagons and dodecagons with the other holes](/images/png/Icosidodecahedron_5FoldSymmetry_3.png) |

## Stats

![A table of statistics on the size of the svg files, including ratio of saved size by culling the image](/images/png/StatTable.png)

## Resources and Acknowledgements

Unless otherwise specified, work here should be considered mine, and mine alone. Instances of these "otherwise specified" cases are:
* `CVector2::ComputeInterpolant()`, which uses [math from Paul Bourke](http://www-cs.ccny.cuny.edu/~wolberg/capstone/intersection/Intersection%20point%20of%20two%20lines.html)
* `CVector2::ComputeOrientation()`, which uses [code from Geeks For Geeks](https://www.geeksforgeeks.org/orientation-3-ordered-points/)
* `CVector2::DoLineSegmentsIntersect()`, which uses [code from Geeks For Geeks](https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/)

I would definitely recomment the use of Geeks For Geeks to people, not only as an introductory aid for c++ as a programming language, but also, more importantly, for their website [ide.geeksforgeeks.org](ide.geeksforgeeks.org). This site has been *invaluable* to me over probably the past ~2 years. Not only is this helpful while "learning" a language, but also whenever there's something little that you want to check, but don't want to create a whole project/file for it, this site is perfect for testing little bits here and there.

— Zeke Baker 2021-04-03 19:38 CDT

## Edit #1

I've made some edits for spelling, grammar, concision, and clarity, and added the [**Gallery**](#gallery) and [**Stats**](#stats) sections. The rest I've left in tact. Knowing what I know now, I feel its worth offering some thoughts on how I'd do things differently if I were to dedicate time to this project again, either as a continuation or from scratch:
* As mentioned in the description, testing for a positive dot product between the positive *z* axis and the normal of a given face could easily rule out half of the faces being considered.
* There's some additional formatting done to the SVG files to make them easier to read/edit in a text editor. Removing this formatting would also reduce file space.
* It has come to my attention that this concept of φ polynomials is fairly similar to the concept of ["base phi numbers"](https://r-knott.surrey.ac.uk/Fibonacci/phigits.html), a concept coined by George Bergman in 1957. Using the information available online on this topic, a "simplest form" algorithm is very likely possible.
* ide.geeksforgeeks.org is basically a simpler form of [godbolt.org](https://godbolt.org/).
* Implementing this in Rust would remove the need for `IntegralTypes.h` and `FloatingTypes.h`.
  * Using the [`glam`](https://docs.rs/glam) crate would remove the need for the `CVector2` and `CVector3` classes.
  * Using the [`clap`](https://docs.rs/clap) crate would make it easier to drive the various functionality exposed in `main.cpp` via command line input.
* Partitioning the possible polygons into 4 sets based on the 4 quadrants of the *xy* plane based on whether or not they had a vertex in that quadrant, then constructing the occlusion mask of each quadrant individually on its own thread would imprmove the overall speed of the culling process.

— Zeke Baker 2022-10-21 18:56 EDT