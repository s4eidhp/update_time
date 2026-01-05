# update_time
A simple program to update the system time of a Linux device using NTP during the boot process.

## Compilation
Compile the C program using GCC:
```
gcc -o update_time update_time.c
```

## Installation
1. Compile the program using make:
   ```
   make
   ```
   Or manually:
   ```
   gcc -o update_time update_time.c
   ```

2. Install the binary:
   ```
   sudo make install
   ```
   Or manually:
   ```
   sudo cp update_time /usr/local/bin/
   ```

3. Edit the root crontab to run at boot with a delay:
   ```
   sudo crontab -e
   ```

4. Add the following line (adjust sleep time as needed, e.g., 30 seconds to ensure internet connection):
   ```
   @reboot sleep 30 && /usr/local/bin/update_time
   ```

## Usage
The program will automatically run at boot with a delay (30 seconds by default) to ensure the internet connection is established. It fetches the current time from an NTP server (pool.ntp.org) and sets the system time.

To check if the cronjob is set:
```
sudo crontab -l
```

To test manually:
```
/usr/local/bin/update_time
```

## Requirements
- GCC for compilation
- Make for building
- Root privileges for setting system time and installing the binary
- Internet connection for NTP

## Makefile targets
- `make`: Compile the program
- `make install`: Install the binary to /usr/local/bin/
- `make clean`: Remove the compiled binary
