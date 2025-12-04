# ParseSNP
This program is a work in progress...
The Project was built in Visual Studio 2022, and because of the C++ 17 settings, you MUST load the Solution once you open the project ie: double-click the SLN file.
Creating a Project will prompt for a project name. This must be alphanumeric only. It, in turn, will become a folder under
C:\Users\<username>\Documents\ParseSNP\Projects
The current DNA file will be copied there, and project entries will be saved.
To load a project, go into that directory and load the ProjectManifest.ptxt
Once a project is created, only the ProjectManifest.ptxt file will be updated when you save.
To delete a project entry double double-click it and you will be prompted to delete.
You can also create 'pathogenics' files from research papers etc. which can then be loaded and run against the currently loaded DNA file to show a risk assessment. At the end of the assessment will be the MD5 hash of the .PPI file loaded. The results can be exported to a .txt file.
From version 0.4., creating a .PPI file will also generate a .MD5 file containing the .PPI file’s MD5 hash. This can be opened with Notepad the MD5 hash is shared with the .PPI file to preserve trust and authenticity.

I included an example for European Type 2 diabetes.
MD5 Hash:31D571C6C3DF9260E04AC81916BBB5F8

Have fun, remember it's a Beta... ;)


thx Robin
