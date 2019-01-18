connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent Nexys Video 210276689540B" && level==0} -index 0
fpga -file /home/akayashima/H30_Xilinx/Multiple-Core-v2/Multiple-Core.sdk/design_1_wrapper_hw_platform_0/design_1_wrapper.bit
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys Video 210276689540B"} -index 0
dow /home/akayashima/H30_Xilinx/Multiple-Core-v2/Multiple-Core.sdk/MicroBlazei0/Debug/MicroBlazei0.elf
bpadd -addr &main
