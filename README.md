# Fake-CSM-Source-2013 Dev mapbase 7.1 branch
Here's an example from a mod I'm currently working on. First Image is lighting using default light_environment, second are my csm.
![image](https://user-images.githubusercontent.com/75926135/224785385-f9101157-030a-471c-8a79-84ba1228c833.png)
![image](https://user-images.githubusercontent.com/75926135/224785413-b9d7660d-7ffc-402d-a4e9-1d221603078c.png)


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
If you have problems with blend materials, try to make it displacement.        
If you have problens like "All CSM are overbright", try to use this(https://www.moddb.com/mods/mapbase/downloads/mapbase-multi-tool) to convert shaders to SDK_ shader from mapbase

And pls add me somewhere in credits or just say about me in the description.

This CSM looks better when `r_flashlightdepthres is 4096` and `mat_slopescaledepthbias_shadowmap is 4`.

# How to install
Go to readme in main branch. 

# If you have some problem or question
then go to the [Discord server](https://discord.gg/DaBHDTwAU7) and describe the problem.
