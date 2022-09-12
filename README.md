# Fake-CSM-Source-2013
Recreated env_cascade_light from CS GO based on env_projectedtexture and Real CSM GMod addon.

- It's not finished yet, but some basic functionality is already there. There are also some issues with lighting orientation, so if you get very strange lighting position behavior then try manually rotating it to the same position as light_environment.

You can change csm settings in console, just type `csm_`.

`If you know how to fix bugs or some visual shit dont be shy and tell me about it`

|    Game    |  Can Work  |
|----------------|---------------:|
|CSS         |     NO     |
|Sdk 2013    |    Yes (tesed for sp)     |


# Features
- Partial compatibility with CSGO.

# How to make this work in your Source 2013 mod?
Copy client and server foldert to your "source code/game" folder, then find client_base/server_base.vpc files in these folders and add `$Include "csm.vpc"` to the top, open createallprojects.bat (or creategameprojects.bat) and add "-f" to the end of the line. After all this stuff you should launch your bat file.

if you can't compile the code then make sure you have lights.h in your project and it has this code:

```cpp
class CEnvLight : public CLight
{
public:
	DECLARE_CLASS(CEnvLight, CLight);

	bool	KeyValue(const char* szKeyName, const char* szValue);
	void	Spawn(void);
};
```

Then copy materials folder to the your mod folder.

Your `mat_depthbias_shadowmap` better should be `0.00001`, `mat_slopescaledepthbias_shadowmap` should be `4` and `r_flashlightdepthres` should be more then `2048`. Its only for better visual and you not literally should do that.

# How to add this light in your map
Copy csm.fgd from fgd folder to bin folder in SDK Base 2013.
Add this fgd to your hammer.
Add env_cascade_light to the map and set it to the angle you want. 
If you want to use the angle from light_environment then you need to set `Use light_environment's angles` to Yes and do some manipulation with your light_environment:
- Add any targetname to your light_environment
- Copy Pitch to second number of the Angle.
- Thats how it should looks like:

![Alt-текст](https://github.com/celisej567/Fake-CSM-Source-2013/blob/main/screen1.png "")

here is example

![Alt-текст](https://github.com/celisej567/Fake-CSM-Source-2013/blob/main/example.png "")

``IF THIS IS WORKS BAD YOU NEED TO FIND RIGHT ANGLE BY YOURSELF!``

# If you have some problem or question
then go to the [Discord server](https://discord.gg/DaBHDTwAU7) and describe the problem.

