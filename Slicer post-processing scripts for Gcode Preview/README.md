Installation of Python : https://www.python.org/downloads/release/python-3101/
While you're in python setup windows set the checkbox add Python 3.10 to PATH. Enable all optional features. Go to Advanced, check add python to environment variables, set install directory C:\Program Files\Python310

To include PrusaSlicer thumbnails:


1- Copy the script 'PrusaSlicer_JPEG_Preview.py' in a folder ie :
C:\Program Files\Prusa3D\Scripts_Post-processings\
2- Open PrusaSlicer and go to Print Settings Tab, Post-processing scripts section and enter exactly in the input field "C:\Program Files\Python310\python.exe" "C:\Program Files\Prusa3D\Scripts_Post-processings\PrusaSlicer_JPEG_Preview.py";
3- Next go to Printer Settings tab, Firmware section and check if you have 50x50, 180x180. If not enter this parameter in the input field.
4- Don't forget to save your new profil.

To include Cura thumbnails :

1- Go to the Help menu on Cura and select Show Configuration Folder in Windows you will get an explorer opened in the path inside of your user profile folder: C:\Users\'user name'\AppData\Roaming\cura\'version_cura'\scripts
2- Close Cura and save the script 'Cura_JPEG_Preview.py' in the scripts folder.
3- Open Cura go to Extensions > Post processing > Modify G-Code -> Add a script - > Create JPEG Preview.
4- If not, check cases Create thumbnail and preview. 
5- Close windows.
If everything is done right you will see little icon next to slicing button which shows that the script is enabled.