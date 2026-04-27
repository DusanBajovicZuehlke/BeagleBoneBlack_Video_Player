# BeagleBone Black Video Output

The BeagleBone Black supports video output through its **tilcdc** (Texas Instruments LCD Controller) driver, which exposes a framebuffer device at `/dev/fb0`. Combined with the onboard **mini-HDMI port**, this makes it possible to display graphical content directly from the board.

## How It Works

The `tilcdc` kernel driver manages the LCD controller hardware and presents it as a standard Linux framebuffer. Any application that can write pixel data to `/dev/fb0` can render content on a connected HDMI display — no full desktop environment required.

## Pipeline Overview

@startuml
start
:User Program (MP4 argument);
-> Decode frames;
:FFmpeg Decoding (CPU decoding);
-> Generate raw pixel data;
:/dev/fb0 (Framebuffer device);
-> Write pixel data;
:tilcdc Driver (Kernel space);
-> Hardware control;
:LCD Controller HW;
-> Signal;
:mini-HDMI Port;
-> HDMI cable;
:HDMI Display;
end
@enduml

## Playing Video with a Custom Program

Current solution for solving this problem:

1. Take an **MP4 file** as a command-line argument
2. Decode the video frames (using FFmpeg libraries)
3. Write raw pixel data directly to `/dev/fb0`

This approach is lightweight and well-suited to the BeagleBone Black's constrained resources and current goals of the team.

> **Note:** Frame rate and resolution will be limited by the board's CPU decoding capacity (scaling the video to match the framebuffer resolution would be the best solution).