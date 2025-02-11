# GenericRenderer

A vulkan-based renderer and game engine research project.

## Purpose

The intent of this project is to become more adept with vulkan, modern graphics concepts, and to create a playground with which to explore shading models and techniques such as PBR shading, subsurface scattering, etc.

It has already become a useful playground for exploring the many ways software can or should be architected in C++, and will eventually pivot into a platform for making games and spatial (AR/VR) applications.

It should eventually become a way to sharpen my expertise with libraries are common for video games (math libraries, audio SDKs, XR SDKs), and other related technologies such as tile-preserving subpasses in Vulkan, fragment density maps that determine tile resolution, and more mobile platform concepts.

## Dependencies
Current:
- Vulkan
- GLM

Anticipated:
- FMOD/Wwise/OpenAL
- OpenXR

## Style Guide

in-line brackets for if, else, and else if statements. Drop-brackets for everything else.
Stay within 100 character lines, aim for 80.
Prefer char16 string literals and types defined in Types.h or stdinc.h 

Class definition structure order:
1. Types
2. Statics and consts
3. All Variables (variables must be private, for direct var access objects use structs)
4. Getters & Setters
5. Public interface methods
6. Private utility methods
7. *tors and operator overrides.