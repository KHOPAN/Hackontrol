# Hackontrol
A tool for controlling the target computers using Discord bot, written in Java and C (Native Library)

### Features
- Support multiple devices (through the Discord category system)
- Notify when target devices are online
- Ping the target devices
- Screen capture (Screenshot)
- Power control (sleep, hibernate, restart and shutdown)
- Microphone streaming (recording and device sound not supported yet)
- Command prompt (in a dedicated Discord channel)
- File browser (support download/upload)
- Key logger!
- Keyboard locking/unlocking
- Dialog or message box (for chatting with the target?)
- Webcam capture

Most (Java) errors are designed to show in the Discord server and not on the target device. But some errors, mainly in C or DLL files, the errors would show up as a message box

### Infection Process
To infect a device with Hackontrol, run the file, it will ask for Administrator, click `Yes` and that's it

### Updating Process
When you install it, Hackontrol will start automatically when the target login, when it starts, it will update automatically. For the Java code, the DLL is packed with the JRE and unpacked when executed, this way the target doesn't have to install Java

**Disclaimer:** *This tool is created for educational purposes only, I will not be responsible for any misuse or damage caused by using this tool!*
