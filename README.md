# How to Build
1) Open OverlappedDragPreview.sln
2) Set the build target platform to x64. 
3) Set the build configuration to either release or debug. The issue is reproducible regardless of the build configuration.
4) Build and run.

# How to reproduce the issue
1) Drag the 1st image, and release mouse button to cancel the dragging operation.
2) Drag the 2nd image
3) You should see the second image on top of the first image in the dragging preview.

# Note
Build and reproduction of the issue were tested with Visual Studio 2017.
