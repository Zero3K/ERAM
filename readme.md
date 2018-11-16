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

## Benchmarks

![ERAM Benchmark done on Windows 7 64-bit](images/benchmark.png)
![ERAM Benchmark done on a Windows 7 64-bit Virtual Machine](images/benchmark_2.png)

## Screenshots

![ERAM's RAM Disk Contents in a Windows 7 64-bit Virtual Machine](images/Empty_ERAM_Drive.png)
![ERAM's RAM Disk Properties in a Windows 7 64-bit Virtual Machine](images/ERAM_Drive_Properties.png)

![ERAM's Control Panel Applet in a Windows 7 64-bit Virtual Machine](images/ERAM_Options.png)

## Original Developer's Website

http://www.vector..jp/authors/VA000363
