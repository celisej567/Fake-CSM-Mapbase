# Fake-CSM-Source-2013 Dev mapbase 7.1 branch
Here's an example from a mod I'm currently working on. First Image is lighting using default light_environment, second are my csm.
![image](https://user-images.githubusercontent.com/75926135/224785385-f9101157-030a-471c-8a79-84ba1228c833.png)
![image](https://user-images.githubusercontent.com/75926135/224788092-1328088d-93b5-47e9-ba48-d0ce7f930776.png)



This branch contains some changes only for mapbase 7.1 and some test shit.

Its very expensive.

Read main branch readme to get more info.

Thanks:        
-sitkinator      
-Xenthio

# Branch Features

-Somehow fixed model lighting problems.

-Changes intensity because mapbase updates.

-Fixed Angle problems.

-You dont need to copy Pitch to Angles in light_env.

-Added light_env changes to csm.fgd.

-Fixed color problems.

-Added Brightness.

-Added Name for light_env.


If you have `Assert server/baseentity.cpp in 7240 line` you should go to this file and comment it. Also comment Warning up there.         
      
If you have problens like "All CSM are overbright", try to use this(https://www.moddb.com/mods/mapbase/downloads/mapbase-multi-tool) to convert shaders to SDK_ shader from mapbase

If you is you still have same problems, try to make it displacement. Blend shaders renders flashlight on different way on diplacements.

And pls add me somewhere in credits or just say about me in the description.

This CSM looks better when `r_flashlightdepthres is 4096` and `mat_slopescaledepthbias_shadowmap is 4`.

# How to install
Go to https://github.com/celisej567/Fake-CSM-Source-2013/wiki

# How to use
This branch contains some differences from main branch:
* Instead of copy-paste angles from ligth_env you just should set a name for light_env (csm dont care what exacly name will be). 
* You should copy color from light_env to CSM color. After this you should change brightness of the light_env (its fourth number in color) to some low value, like 50 or 20.

# If you have some problem or question
then go to the [Discord server](https://discord.gg/DaBHDTwAU7) and describe the problem.
