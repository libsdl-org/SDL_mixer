
# Introduction to SDL_mixer with Visual Studio

The easiest way to use SDL_mixer is to include it along with SDL as subprojects in your project.

We'll start by creating a simple project to build and run [hello.c](hello.c)

- Create a new project in Visual Studio, using the C++ Empty Project template
- Add hello.c to the Source Files
- Right click the solution, select add an existing project, navigate to the SDL VisualC/SDL directory and add SDL.vcxproj
- Right click the solution, select add an existing project, navigate to the SDL_mixer VisualC directory and add SDL_mixer.vcxproj
- Select your SDL_mixer project and go to Project -> Add Reference and select SDL3
- Select your SDL_mixer project and go to Project -> Properties, set the filter at the top to "All Configurations" and "All Platforms", select VC++ Directories and modify the default SDL path in "Include Directories" to point to your SDL include directories
- Select your main project and go to Project -> Add Reference and select SDL3 and SDL3_mixer
- Select your main project and go to Project -> Properties, set the filter at the top to "All Configurations" and "All Platforms", select VC++ Directories and add the SDL and SDL_mixer include directories to "Include Directories"
- Build and run!

Support for GME, Ogg, Opus, Wavepack, and XMP are dynamically loaded at runtime. You can choose to include those DLLs and license files with your application if you want support for those formats, or omit them if they are not needed.
