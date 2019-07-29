# Things to do (mostly ordered)

## kernel_task / libSysCall
1. Add Thread model for processes
2. Pass ACPI caps to DriverKitD
3. Create Endpoints communication between programs
4. Create Client/Server framework

## DriverKitD / libDriverKit
Create a driver model, with IRQ caps and port access.

## Terminal/Serial port
5. Write a Terminal Server, and spawn one instance by DriverKitD of each com port found
6. Spawn a Shell for each Terminal Server.

## VFSD
1. Create a server that acts like a router for all the file-sytem related syscalls. This server has a registration system to add file systems.
2. Write an ATA driver, spawned by DriverKitD, and connected to VFSD.
