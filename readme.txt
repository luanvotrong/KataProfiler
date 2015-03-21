======================================================
*** KataProfiler ***
======================================================

This tool is similar to AdrenoProfiler but it can be run on all GPU (not only Adreno).

It contains 2 part: Server and Client.

======================================================
Server
======================================================

Server is a static library (.a file), so you can link it to your program (game).

* Compile:
	- Android: go to Server\projects\android, run m.bat. Open m.bat to set your ANDROID_NDK_HOME path,
					also make sure you have Cygwin bin folder in your Path environment var.
					
	- Tizen: go to Server\projects\tizen, run m.bat. Make sure you have a environment var called TIZEN_SDK point to your Tizen SDK path,
				or you can set it in Win2Tiz.xml directly.
				
	- Win32: just for edit source only (Server\projects\win32\KataProfiler_Server.sln).
				
* Use:
	- Link the .a file (libkataprofiler.a) to your OpenGL ES program.
	- After eglSwapBuffers() in your program, insert this code: void KPSwapBuffers(); KPSwapBuffers();
			in case you have C code, insert: void C_KPSwapBuffers(); C_KPSwapBuffers();
			
======================================================
Client
======================================================

Client is a Windows Form Application written in C#.

Open the VS solution: Client\KataProfiler.sln.
