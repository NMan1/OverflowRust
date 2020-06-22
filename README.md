# Overflow Rust

Bypass hooks an imported function inside dxgkrnl.sys. This imported function resides in watchdog.sys, and is called when NtDxgkCreateTrackedWorkload gets called from win32u.dll

NtDxgkCreateTrackedWorkload syscall win32u.dll -> NtDxgkCreateTrackedWorkload dxgkrnl.sys -> (half way through function) WdLogEvent5_WdError watchdog.sys

<img src="https://i.imgur.com/gpx81z5.png">/

This hook could be detcted if two things occur
1. If EAC scans watchdog.sys for hooks (%99 doubt they do, its just some random windows non-pg protcted module why would they)
2. If EAC scans and compares all loaded drivers (doubt too would cause to many false positives probaly?)

To pass the struct I used shared memory. I think the bypass is safe, although the renderering method is probaly not. 
Method I used to render: https://github.com/thesecretclub/window_hijack

# Features:
- Recoil Control
- Auto Pistol
- Spiderman
- Admin flags
- ESP
  - Player
  - Scientist
  - Stash
  
  
# Pictures:
<img src="https://i.gyazo.com/25733304ff78fb87490a3412a4b75b84.png">/
<img src="https://i.gyazo.com/f5108bd755460600c049bb06e79d4119.jpg"/>
<img src="https://i.gyazo.com/053775f4c5ddf691203bdf6804fb77ea.jpg"/>


# Credits:
- Me
- Window Hijacking https://github.com/thesecretclub/window_hijack
 - Hooking class/library: https://github.com/adrianyy/kernelhook
