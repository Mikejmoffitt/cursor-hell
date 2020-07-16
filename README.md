Cursor Hell
===========

This project was supposed to be a tiny proof-of-concept demo showing how a low-latency windowed Vsynced game can be realized in Windows 10 by abusing the mouse cursor as a rendering surface. The intent is to bypass some seemingly inescapable layers of buffering that Windows 10 and/or the NVIDIA driver are imposing, even with "Ultra-low latency" enabled.

The game is rendered to a buffer on the video card, and the final result is copied to the mouse cursor buffer. Then, we wait for Vsync like normal. The mouse cursor (on my NVIDIA card/driver combo) seems to double buffer the cursor, and swaps at vblank, so the frame we just finished is immediately visible on the next frame.
