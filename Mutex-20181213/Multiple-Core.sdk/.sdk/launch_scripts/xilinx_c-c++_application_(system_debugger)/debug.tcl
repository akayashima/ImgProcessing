connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
rst -system
after 3000
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
dow /home/akayashima/H30_Xilinx/ImgProcessing/Mutex-20181213/Multiple-Core.sdk/MicroBlazei0/Debug/MicroBlazei0.elf
targets -set -nocase -filter {name =~ "microblaze*#1" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
dow /home/akayashima/H30_Xilinx/ImgProcessing/Mutex-20181213/Multiple-Core.sdk/MicroBlazei1/Debug/MicroBlazei1.elf
bpadd -addr &main
