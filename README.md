# Building GunZ SRC
- Use [SRC Base 2019](https://github.com/ZER0NIS/Gunz-Base) and updated, edit, custom by Century VN.
- [Client files](https://drive.google.com/file/d/1cacpZVDCj_dhlwLQy8iBXVyojYUJPOqA/view?usp=sharing)

## Windows:
Requisites:

* Visual Studio 2019 withATL/MFC support. You can download the free Community edition at https://www.visualstudio.com/downloads/. In the installer, select the "Desktop development with C++," and on the right under "Summary," select "MFC and ATL support (x86 and x64)." Picture here).

## Build
	- Build with `USA_Release_Publish`.
	- Search around SRC `// Edit` or `// Fix` to see all my updated.

### TODO'S
* Done
	- Fix toggle screen, resolution, widescreen....
	- Edit Custom I, II ... Option -> Video
	- Edit people in room (1,2,4,6,8,10,12,14,16).
	- Edit check box show hpap, exp, fullscreen.
	- Edit taunt commands working after round is over, fix spawn the duel.
	- Fix optimize windowed mode fps.
	- Fix hell climb.
	- Fix show true online user.
	- Edit show map name in roomlist.
	- Edit show admin name (root, developer, event master, administrator).
	- Fix checkbox remember password, auto select server.
	- Fix server mode: clan.
	- Edit character visibilty.
	- Edit tag [name] = ESP, [ia] = infinite ammo, [hpap] = 150HP/150AP, [sgo] = 120HP/120AP, [spo] = 150HP/150HP, [ns] = no sword, [ni] = no item, [ng] = no gun,....
	- Ctrl F, G to show hpap, fps.
	- Edit hide mode survival.
	- Fix limit item.
	- Fix team Deathmatch and team games with solo game spawns.
	- Edit login to character.
	- Edit show duel match map only map.xml `<bOnlyDuelMap = "true">`.
	- Fix limit FPS. 
	- Fix shop jump.
	- Remove camera scroll for staff


* Undone, bug, ...
	- Tag [r] (no reload) not work. Some tag room not add now.
	- Not add new command.
	- Not fix right click sniper.
	- Add new grade Vip 1 - 15 done. Bug show name (when it vip, admin,...) in lobby and wait room.
	- Not show shop in wait room.
	- DB clean not fix. only edit more VIP.
	And more....