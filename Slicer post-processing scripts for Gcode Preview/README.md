Installation of Python : https://www.python.org/downloads/release/python-3910/
While you're in python setup windows set the checkbox add Python 3.10 to PATH. Enable all optional features. Go to Advanced, check add python to environment variables, set install directory C:\Program Files\Python39

BE CAREFULL : Don't use Space ' ' or other '.' for the file name !

To include PrusaSlicer/SuperSlicer thumbnails:

1- Copy the script 'PrusaSlicer_JPEG_Preview.py' (for PrusaSlicer <= v2.3.3) or the script 'PrusaSlicer_JPEG_Preview_P24x.py' (for PrusaSlicer >= v2.4.x) in a folder ie : C:\Program Files\Prusa3D\Scripts_Post-processings\
2- Open PrusaSlicer and go to Print Settings Tab, Post-processing scripts section and enter exactly in the input field for PrusaSlicer <=2.3.3 : "C:\Program Files\Python39\python.exe" "C:\Program Files\Prusa3D\Scripts_Post-processings\PrusaSlicer_JPEG_Preview.py";  or for PrusaSlicer >=2.4.x : "C:\Program Files\Python39\python.exe" "C:\Program Files\Prusa3D\Scripts_Post-processings\PrusaSlicer_JPEG_Preview_P24x.py";

Or use the .exe scripts (PrusaSlicer_JPEG_Preview.exe for PrusaSlicer <=2.3.3) and (PrusaSlicer_JPEG_Preview_P24x.exe for PrusaSlicer >= 2.4.x).
You have to copy the scripts to the same folder as prusaslicer.exe or superslicer.exe is on, then on "Post processing scripts", place ONLY the following PrusaSlicer_JPEG_Preview.exe or PrusaSlicer_JPEG_Preview_P24x.exe

3- Next go to Printer Settings tab, Firmware section and check if you have 50x50, 180x180. If not enter this parameter in the input field.
4- Don't forget to save your new profil.

To include Cura thumbnails :

1- Go to the Help menu on Cura and select Show Configuration Folder in Windows you will get an explorer opened in the path inside of your user profile folder: C:\Users\'user name'\AppData\Roaming\cura\'version_cura'\scripts
2- Close Cura and save the script 'Cura_JPEG_Preview.py' in the scripts folder.
3- Open Cura go to Extensions > Post processing > Modify G-Code -> Add a script - > Create JPEG Preview.
4- If not, check cases Create thumbnail and preview. 
5- Close windows.
If everything is done right you will see little icon next to slicing button which shows that the script is enabled.