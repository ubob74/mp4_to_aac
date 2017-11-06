1. Description

The program grabs audio track from MP4 video file
and stores it to AAC-coded file.


2. Install

Move to the program root directory and type commands:
 - cd ./src
 - make
 - cd ../bin


3. Usage

Put to the command string an MP4 video file to be converted:

 ./mp4_to_aac <MP4-coded file>

The output file ./audio_track.aac will be formed at the current directory.


4. Restrictions

The program works correctly with a bitrate 44100Hz.

