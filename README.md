# SofaRHI

## Rendering Hardware Interface for SOFA

This plugin brings Rendering Hardware Interface API (from Qt) into SOFA.
Concretly, it allows the user to execute rendering code using one of those API: OpenGL ES 2, Vulkan, Direct3D 11 and Metal, with the same code.
It is relying on the RHI API from Qt (>5.12).

Note that this API is still considered private in Qt5. (meaning no support from them and the code can break anytime).

This plugin is composed of :
 - SOFA components to draw using RHI (VisualModel, VisualLoop)
 - a Drawtool class for the existing code (mainly in the draw() functions)
 - a viewer meant for runSofa and its Qt interface (effectively replacing QtViewer and QGLViewer)
 - an offscreen renderer

## Important notes

This plugin is in beta version and not all functions in DrawTool are implemented.

Furthermore, DrawTool is not optimized (at all) and therefore it could run slower than the 2 other viewers.
Neverthless in pure Visual rendering (i.e Visualmodels), expect 10-20% faster rendering than QtViewer/QGLViewer.

This plugin is totally incompatible with the existing OpenGL Code; once loaded, this plugin will disable all OpenGL components from SofaOpenGlVisual.
(OglModel will be replaced by RHIModel, all other OpenGL-related components are replaced with a "DisabledObject")

The choice of the underlying system is set according to the OS:
 - Windows: Direct3D 11
 - macOS: Metal
 - Linux: OpenGL

 Vulkan is totally not tested for the moment.

## How to build
This can be built as any SOFA plugin.

The only peculiar part is the Viewer/runSofa; this plugin needs to be loaded before the GUI is set up.
This can only be done either:
 - with the autoloaded file (default `plugin_list.conf.default`)
 - or with the commandline: `-l SofaRHI`

## How to run
### Viewer
`./runSofa.exe <YOUR_SCENE> -g rhi`

### Offscreen renderer 
`./runSofa.exe <YOUR_SCENE> -g rhi_offscreen` 

## TODO
- commandline parameters (numbers of iterations for rhi_offscreen, choice of graphic API) -> order problem with parser and runSOFA
- add implementations in the DrawTool
- add ComputeBuffer use (WIP)
- custom lighting 💡
- many things 🙃

## Licence
LGPLv3 (see LICENCE)
