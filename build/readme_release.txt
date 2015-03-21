======================================================
*** KataProfiler ***
======================================================

This tool is similar to AdrenoProfiler but it can be run on all GPU (not only Adreno).

It contains 2 parts: Server and Client.

======================================================
Server
======================================================

Server is a static library (.a file).
	- Link libkataprofiler.a to your OpenGL ES program.
	- At the end of a frame (end of update/render function, after eglSwapBfufers, ...) in your program, insert this C++ code: void KPSwapBuffers(); KPSwapBuffers(); in case you have C code, insert C code: void C_KPSwapBuffers(); C_KPSwapBuffers(). If you are working on Android, you should insert into onDrawFrame() function; since onDrawFrame() is Java code, you should use JNI to insert C/C++ code.
			
======================================================
Client
======================================================

Client is a Windows Form Application written in C#. Just open KataProfiler.exe :)
