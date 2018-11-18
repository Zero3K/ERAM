# ERAM

ERAM v2.23 is an Opensource RAM Disk made by Hideaki Okubo (okubo at msh.biglobe.ne.jp) with the source code comments translated by Katayama Hirofumi MZ (katayama.hirofumi.mz at gmail.com). It has a size limit of 4 GB that uses page/non-paged/external RAM. You can use it for storing temp files, browser cache, etc. in order to speed up the programs that use those files.

## Install Instructions

x86
---
1. Copy eramnt.sys from the x86 directory to \Windows\System32\drivers.
2. Import eramnt.reg from the x86 directory to the Registry.
3. Restart

x64
---
1. Copy eramnt64.sys from the x64 directory to \Windows\System32\drivers
2. Import eram64.reg from the x64 directory to the Registry.
3. Restart

After installing, the RAM Disk will be available as the R: drive with a size of 996 MB.
You can use the included eramnt.cpl (in the x86 directory) / eram64.cpl (in the x64 directory) to change the size, etc. of the RAM Disk.

## Build Instructions

1. Download WDK (Windows Driver Kit) 7.1.0 from https://www.microsoft.com/en-us/download/details.aspx?id=11800.
2. Extract the ISO using 7-Zip, etc. to a directory of your choosing.
3. Run the KitSetup.exe in the directory it was extracted to.
4. Click Full Development Environment.
5. Click OK.
6. After the install has completed, you can find the Build Environments in the Start Menu in Windows Driver Kits\WDK 7600.16385.1\Build Environments.
7. Open the proper Environment depending on what OS and CPU architecture you are building for (Checked makes a debug build while Free makes a release build).
8. Change the directory to the location of the source code (for example, cd C:\ERAM).
9. Type build and press Enter.
10. The resulting driver is unsigned so Driver Signature Enforcement has to be disabled for it to load in x64 architecture OSes.

## Benchmarks

![ERAM Benchmark done on Windows 7 64-bit](images/benchmark.png)
![ERAM Benchmark done on a Windows 7 64-bit Virtual Machine](images/benchmark_2.png)

## Screenshots

![ERAM's RAM Disk Contents in a Windows 7 64-bit Virtual Machine](images/Empty_ERAM_Drive.png)
![ERAM's RAM Disk Properties in a Windows 7 64-bit Virtual Machine](images/ERAM_Drive_Properties.png)

![ERAM's Control Panel Applet in a Windows 7 64-bit Virtual Machine](images/ERAM_Options.png)

## Original Developer's Website

http://www.vector.co.jp/authors/VA000363
