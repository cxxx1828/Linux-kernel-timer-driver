# Linux Kernel Timer Driver 

## Overview

This project implements a custom Linux kernel platform driver that enables time-controlled string output using the Raspberry Pi system timer. The driver integrates interrupt handling, blocking I/O, and device tree configuration to provide a controlled interaction between user space and kernel space.

The core functionality allows a user to write a string to the driver, after which a blocking read operation waits for a timer interrupt before returning the stored data.

---

## Objectives

- Implement a Linux kernel platform driver
- Configure and use the Raspberry Pi system timer via Device Tree
- Enable interrupt-driven execution
- Support blocking read operations using wait queues
- Provide user-space interaction through a misc character device
- Ensure proper synchronization of shared resources

---

## Functional Description

The driver operates as follows:

1. A string is written to the device using the `write()` system call
2. The `read()` system call blocks until a timer interrupt occurs
3. The timer interval is configured via `ioctl()`
4. When the timer reaches the configured value, an interrupt is generated
5. The interrupt handler:
   - Clears the interrupt status
   - Updates the compare register for the next interval
   - Wakes up the blocked process
6. The `read()` call returns the stored string to user space

---

## System Architecture

The implementation consists of the following components:

- **Platform Driver**  
  Responsible for binding the driver to the hardware defined in the Device Tree

- **Misc Character Device**  
  Provides a simple interface for user-space interaction (`/dev/timer`)

- **Interrupt Handler**  
  Handles timer interrupts and coordinates execution flow

- **Wait Queues**  
  Used to block and wake processes during read operations

- **Synchronization Mechanisms**  
  Spinlocks and/or mutexes are used to protect shared resources

---

## Device Tree Configuration

A custom Device Tree Source (DTS) file is created based on the Raspberry Pi 2 configuration:

```

bcm2709-rpi-2-b-custom.dts

```

The following properties are defined:

- Timer node within the `soc` node
- Base address and register size (`reg`)
- Interrupt lines (TIMER0–TIMER3)
- Clock frequency (1 MHz)
- Compatible string for driver binding

---

## Project Structure

```

linux-kernel-labs/
├── modules/
│   └── timer/
│       ├── timer.c
│       ├── Makefile
├── dts/
│   └── bcm2709-rpi-2-b-custom.dts
├── README.md

````

---

## Build and Execution

### Build the module

```bash
make
````

### Insert the module into the kernel

```bash
insmod timer.ko
```

### Remove the module

```bash
rmmod timer
```

---

## Usage

### Write a string to the device

```bash
echo "Test string" > /dev/timer
```

### Read from the device (blocking)

```bash
cat /dev/timer
```

The output will be returned only after the configured timer interval expires.

---

## Synchronization and Concurrency

Due to the concurrent execution between interrupt context and user-space operations, shared resources must be properly protected. This is achieved using synchronization primitives such as spinlocks or mutexes, ensuring data consistency and preventing race conditions or deadlocks.

---

## Key Concepts

* Linux Kernel Modules
* Platform Drivers
* Misc Devices
* Interrupt Handling
* Wait Queues
* Device Tree
* Memory-Mapped I/O

---

## Notes

* The Linux kernel source is not included in this repository due to its size and availability as an external dependency
* Only custom driver code and configuration files are provided

---

## Author

Nina Dragićević
Faculty of Technical Sciences, Novi Sad
Department of Computer Engineering and Communications

---

## License

This project is intended for educational purposes only.

