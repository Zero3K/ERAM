ERAM for Windows NT / 2000 / XP v2.23 Preliminary
2004.7.11 error 15

Part from v2.20,21 is modified version. v2.22 is a modified version of.

It is not in the official version. Read this document,
Please do not use those who do not know well.
Who support not technical details, we can not do.

· Over4GB-time environment in PAE NOLOWMEM specified, specified MAXMEM = n is in over4GB area
Modify the consideration leakage of that work (was going) v2.23 or later

If you NOLOWMEM specified in over4GB environment, to limit the starting position also over4GB position of MAXMEM = n
Move. However v2.21 and earlier, only the RAM in the range of n ~ 4095MB for safety measures
And put a check that does not use, is the memory actually available in the NOLOWMEM environment
It had become something less.
So, at the time of PAE NOLOWMEM specified in the flow, such as if MAXMEM = 17 is specified
I tried to add a control, such as fall. Since the number MB of the top of the 4GB space is used for PCI
Available about real 4000MB memory ... might.
Since the hand there is no over4GB environment, I do not know whether or not actually move.

We had a fine information to Ten'yume forest Nagareirodori's. Thank you very much.
Wall per 1.7GB is at least Ten'yume forest Nagareirodori's environment has been surpassed in the v2.21 time.

This new driver, you can install the time being in a conventional INF file.

It should be noted that, for the behavior at the time of NOLOWMEM specified without PAE is still in v2.23 time
We misidentified the MAXMEM = n, but solution is unknown for now.

And exclusion of the ACPI of memory (going to) v2.21 or later

ACPI NVS, ACPI Reclaim is placed on top of the physical RAM by the ACPI BIOS.
MAXMEM If not specified, these regions Loader Reserved next, from the OS
Will not be used, but, at the time of MAXMEM = n specify becomes the OS unmanaged,
You had to manage on their own.
However, as the ACPI of the environment is almost standard, also information to the effect that it does not work properly
It has increased.
Although ACPI of information must be acquired by the so-called E820 I / F in the realistic mode
Because from the NT-based driver can not call the BIOS, from the table in the memory
It was to guess method.
Table content is going to follow the ACPI BIOS specification 2.0c (2003/8/25).

But there is no ACPI machines are only two on hand.
Okay at hand two, but whether all right to the general public is unknown.

Even then, by reducing the capacity of the RAM disk so as not to overlap with ACPI region
The problem does not occur.

This new driver, you can install the time being in a conventional INF file.

- Two eyes of the corresponding to the driver installation (provisional) v2.21 or later

Because there was a desire to try two put on trial, two eyes forced the
I tried to be able to install.

The new driver is the one first and two second is common, but the file name is in eram2.sys
Become. Also driver name will Eram2. Windows2000 and later, such as the one first
Rather than using the hardware wizard, it will be to the right-click of the INF.
INF file, but it is placed under the 2ND directory, to avoid the hazard
We changed the IN_ and extension.
Return the extension
Install the right-click
Reboot

[Important]
Because of the preliminary version, you will not be able to change the settings of Eram2 from the screen.
And a modification of the set of Eram in the reference, it will touch the registry.

[Important]
Because of the preliminary version, there is no uninstaller.
It will be turned off and search for Eram2 from the registry.
Files will be erased the eram2.sys.
