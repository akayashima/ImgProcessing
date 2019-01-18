connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
rst -system
after 3000
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
dow /home/akayashima/H30_Xilinx/NexysVideoDMA_20181101/proj/DMA.sdk/MicroBlaze_i0/Debug/MicroBlaze_i0.elf
targets -set -nocase -filter {name =~ "microblaze*#1" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
dow /home/akayashima/H30_Xilinx/NexysVideoDMA_20181101/proj/DMA.sdk/MicroBlaze_i1/Debug/MicroBlaze_i1.elf
bpadd -addr &main
