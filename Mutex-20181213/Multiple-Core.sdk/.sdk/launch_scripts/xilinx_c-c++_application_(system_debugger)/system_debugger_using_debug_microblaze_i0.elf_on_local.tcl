connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
dow /home/akayashima/H30_Xilinx/Multiple-Core/Multiple-Core.sdk/MicroBlaze_i0/Debug/MicroBlaze_i0.elf
bpadd -addr &main
