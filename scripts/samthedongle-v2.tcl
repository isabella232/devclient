reset_config trst_and_srst
set CPUTAPID 0x5ba00477
set DBGBASE 0x80030000
set CHIPNAME at91sama5d
set TARGETNAME $CHIPNAME.cpu
set ENDIAN little

source [find mem_helper.tcl]
source [find target/swj-dp.tcl]

swj_newdap $CHIPNAME cpu -irlen 4 -ircapture 0x1 -irmask 0xf -expected-id $CPUTAPID -ignore-version
dap create $CHIPNAME.dap -chain-position $CHIPNAME.cpu
target create $TARGETNAME cortex_a -dap $CHIPNAME.dap -endian $ENDIAN -coreid 0 -dbgbase $DBGBASE

adapter_nsrst_delay 300
jtag_ntrst_delay 200

$TARGETNAME configure -work-area-virt 0 -work-area-phys 0x00200000 -work-area-size 0x20000 -work-area-backup 1
