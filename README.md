# Project Name

Mandelbrot Set Image Generator

## Project Description

This project is a personal initiative to explore graphical programming using SDL (Simple DirectMedia Layer). It involves generating and visualizing the Mandelbrot set, a famous fractal, through a custom image generation algorithm. The project includes two main components: the image generator and the Mandelbrot set diver.

## Installation Instructions

1. **Clone the Repository**

   ```bash
   git clone https://github.com/yourusername/mandelbrot-image-generator.git
   cd mandelbrot-image-generator
   ```

2. **Install Dependencies**

   Ensure you have the SDL library installed on your system. You can install it using your package manager. For example, on Ubuntu:

   ```bash
   sudo apt-get install libsdl2-dev
   ```

   On MacOS, you can use Homebrew:

   ```bash
   brew install sdl2
   ```

3. **Compile the Project**

   Use the following command to compile the project:

   ```bash
   gcc -o MandelbrotDiver MandelbrotDiver.c ImageGenerator.c -lSDL2
   ```

## Usage

1. **Run the Mandelbrot Set Generator**

   Execute the compiled program to generate and visualize the Mandelbrot set:

   ```bash
   ./MandelbrotDiver
   ```

2. **Adjust Parameters**

   You can modify the parameters in the source files to explore different aspects of the Mandelbrot set, such as zoom level, iteration depth, and color schemes.

## Dependencies

- SDL2 (Simple DirectMedia Layer 2)
- GCC (GNU Compiler Collection) or any C compiler
