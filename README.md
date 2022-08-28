# Fake-CSM-Source-2013
Recreated env_cascade_light from CS GO based on env_projectedtexture and Real CSM GMod addon.

-It's not finished yet, but some basic functionality is already there.

# Features
-Partial compatibility with CSGO.

# How to make this work in your Source 2013 mod?
Copy client and server foldert to your "source code/game" folder, then find client_base/server_base.vpc files in these folders and add "$Include "csm.vpc"" to the top, open createallprojects.bat (or creategameprojects.bat) and add "-f" to the end of the line. After all this stuff you should launch your bat file.


