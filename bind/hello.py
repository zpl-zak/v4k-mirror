import os
from v4k import v4k

v4k.cook_config(b'../tools/cook.ini')
v4k.window_create(75.0, v4k.WINDOW_MSAA2)
v4k.window_title(b'hello Python')
cam = v4k.camera()
while v4k.window_swap():
    v4k.ddraw_grid(0)

os._exit(0)