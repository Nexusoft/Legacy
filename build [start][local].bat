taskkill /F /IM Nexus.exe
cd /d E:\Nexus\nexus-development\
timeout /t 1
:BEGIN
del "release\Nexus.exe"
mingw32-make -f makefile.mingw USE_UPNP=1
if not exist "release\Nexus.exe" ( 
pause
GOTO BEGIN 
) else (
   strip release\Nexus.exe
   release\Nexus.exe -debug -printtoconsole -logtimestamps -stake
)