-- this will run on vanilla luajit.exe, provided that v4k.dll and this file are all present in same folder

local v4k=require('v4k')

-- specify location of cookbook
v4k.cook_config("../tools/cook.ini");

-- create 75% sized + MSAAx2 anti-aliased window
v4k.window_create(75.0, v4k.WINDOW_MSAA2)

-- set window title
v4k.window_title("hello luajit")

-- config girl
local girl = v4k.model('kgirl/kgirls01.fbx', 0)
local girl_frame = 0
local girl_pivot = v4k.mat44()
v4k.rotationq44(girl_pivot, v4k.eulerq(v4k.vec3(0,0,0)))
v4k.scale44(girl_pivot, 2,2,2)

-- config & play music
local music = v4k.audio_stream('larry.mid') -- 'wrath_of_the_djinn.xm'
v4k.audio_play(music, 0);

-- config camera
local cam = v4k.camera()

-- main loop
while v4k.window_swap() == 1 and v4k.input(v4k.KEY_ESC) == 0 do
   -- fps camera
   local grabbed = v4k.input(v4k.MOUSE_L) == 1 or v4k.input(v4k.MOUSE_R) == 1
   v4k.window_cursor( v4k.ui_active() == 1 or v4k.ui_hover() == 1 and 1 or (not grabbed) )
   if( v4k.window_has_cursor() ~= 1 ) then
      local wasdec3 = v4k.vec3(v4k.input(v4k.KEY_D)-v4k.input(v4k.KEY_A),v4k.input(v4k.KEY_E)-(v4k.input(v4k.KEY_C)),v4k.input(v4k.KEY_W)-v4k.input(v4k.KEY_S))
      local look2 = v4k.scale2(v4k.vec2(v4k.input_diff(v4k.MOUSE_X), -v4k.input_diff(v4k.MOUSE_Y)), 0.2)
      local move3 = v4k.scale3(wasdec3, cam.speed)
      v4k.camera_moveby(cam, wasdec3)
      v4k.camera_fps(cam, look2.x,look2.y)
   end

   -- draw grid/axis
   v4k.ddraw_grid(0)
   v4k.ddraw_flush()

   -- animate girl
   local delta = v4k.window_delta() * 30 -- 30fps anim
   girl_frame = v4k.model_animate(girl, girl_frame + delta)

   -- draw girl
   v4k.model_render(girl, cam.proj, cam.view, girl_pivot, 0)

   -- showcase ui
   if v4k.ui_panel("luajit", 0) == 1 then
      v4k.ui_panel_end()
   end
end
