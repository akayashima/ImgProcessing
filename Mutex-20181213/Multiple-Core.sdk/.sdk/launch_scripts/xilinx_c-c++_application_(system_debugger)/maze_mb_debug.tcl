connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent Nexys Video 210276689540B" && level==0} -index 0
fpga -file /home/akayashima/Documents/Nexys-Video-DMA/proj/DMA.runs/impl_1/design_1_wrapper_cache.bit
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
rst -system
after 3000
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
dow /home/akayashima/Documents/Nexys-Video-DMA/proj/DMA.sdk/maze_mb/Debug/maze_mb.elf
bpadd -addr &main
