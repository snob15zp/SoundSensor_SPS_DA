echo off
del /q projects\target_apps\dsps\dsps_device\Keil_5\out_531\*
del /q out\*

set /p _fw=<version.txt
set _hdr=#define SDK_VERSION ^"%_fw%

set _version=%_hdr%-A"
echo %_version%
echo %_version%>projects\target_apps\dsps\dsps_device\src\version.h

echo "Build image A (0x4000)"
c:\Keil_v5\UV4\uv4 -b projects\target_apps\dsps\dsps_device\Keil_5\dsps_device_ss.uvprojx
copy projects\target_apps\dsps\dsps_device\Keil_5\out_531\dsps_device_531.hex out\dsps_device_531.img-A.hex
utilities\hex2bin.exe out\dsps_device_531.img-A.bin out\dsps_device_531.img-A.hex
utilities\mkimage.exe single out\dsps_device_531.img-A.bin projects\target_apps\dsps\dsps_device\src\version.h out\dsps_device_531.A.img

set _version=%_hdr%-B"
echo %_version%
echo %_version%>projects\target_apps\dsps\dsps_device\src\version.h

echo "Build image B (0x1F000)"
c:\Keil_v5\UV4\uv4 -b projects\target_apps\dsps\dsps_device\Keil_5\dsps_device_ss.uvprojx
copy projects\target_apps\dsps\dsps_device\Keil_5\out_531\dsps_device_531.hex out\dsps_device_531.img-B.hex
utilities\hex2bin.exe out\dsps_device_531.img-B.bin out\dsps_device_531.img-B.hex
utilities\mkimage.exe single out\dsps_device_531.img-B.bin projects\target_apps\dsps\dsps_device\src\version.h out\dsps_device_531.B.img

echo "Build secondary bootloader"
c:\Keil_v5\UV4\uv4 -b utilities\secondary_bootloader\secondary_bootloader.uvprojx
copy D:\wrk\embd\SoundSensor_SPS_DA\utilities\secondary_bootloader\out_DA14531\Objects\secondary_bootloader_531.hex out\secondary_bootloader_531.hex
utilities\hex2bin.exe  out\secondary_bootloader_531.bin out\secondary_bootloader_531.hex

echo "Create release file"
utilities\mkimage.exe multi spi out\secondary_bootloader_531.bin out\dsps_device_531.A.img 0x4000 out\dsps_device_531.B.img 0x1f000 0x38000 out\dsps_device_531.release.img