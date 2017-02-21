#=====================================
#《基于网络仿真器NS2的Ad Hoc网络路由协议仿真》一文中的例子
# Define options
#=====================================
set val(chan)		Channel/WirelessChannel ;# channel type
set val(prop) 		Propagation/TwoRayGround ;# radio-propagation model
set val(netif)		Phy/WirelessPhy ;
set val(mac)		Mac/802_11 ;
set val(ifq)		Queue/DropTail/PriQueue ;
set val(ll)		LL ;
set val(ant)		Antenna/OmniAntenna ;
set val(ifqlen)		20 ;
set val(nn)		50 ;
set val(rp)		DSR;# routing protocol
set opt(cp)		"cbr-50n-10c-4p" ;
set opt(sc)		"scene-50n-0p-20M-900t-1000-1000" ;
set val(atk)            20;# number of attackers 
set val(stop)		900 ;# time of simulation end
set val(x)		1000 ;# X dimension of topography
set val(y)		1000 ;# Y dimension of topography

Mac/802_11 set dataRate_ 2e+6
#for 150m radio transmission range
#Phy/WirelessPhy set RXThresh_ 2.81838e-09


Agent/DSRAgent set isActive 0
Agent/DSRAgent set isBlackhole 0
Agent/DSRAgent set isGrayhole 0

# ------------------------------------------------------------------------------
# Fixing DSR bug
# ------------------------------------------------------------------------------
if {$val(rp) == "DSR"} {
	set val(ifq) CMUPriQueue
}

#===============================
# Main Program
#===============================
set ns_ [new Simulator]
set tracefd [open Ablkdsr.tr w]
$ns_ trace-all $tracefd
#$ns_ use-newtrace
set namtracefd [open Ablkdsr.nam w]
$ns_ namtrace-all-wireless $namtracefd $val(x) $val(y)
set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)
set god_ [new God]
create-god $val(nn)

$ns_ node-config -adhocRouting $val(rp) \
		-llType $val(ll) \
		-macType $val(mac) \
		-ifqType $val(ifq) \
		-ifqLen $val(ifqlen) \
		-antType $val(ant) \
		-propType $val(prop) \
		-phyType $val(netif) \
		-channelType $val(chan) \
		-topoInstance $topo \
		-agentTrace ON \
		-routerTrace ON \
		-macTrace OFF \
		-movementTrace OFF \

for {set i 0} {$i < $val(nn)} {incr i} {
set node_($i) [$ns_ node]
$node_($i) random-motion 0
}

#Generate Random interger by CRJ
proc RandomRange { min max } {
set rd [expr rand()]
set result [expr $rd * ($max - $min) + $min]
return $result
}

proc RandomRangeInt { min max } {
    return [expr int([RandomRange $min $max])]
}
#ended by CRJ

for {set i 22} {$i < (22+$val(atk))} {incr i} {
set num [RandomRangeInt 0 3]
if { $num == 1 } {
set ragent [$node_($i) set ragent_]
$ragent set isBlackhole 1
$ragent set isActive 1
$ragent set isGrayhole 0
} elseif { $num == 2 } {
set ragent [$node_($i) set ragent_]
$ragent set isBlackhole 1
$ragent set isActive 0
$ragent set isGrayhole 1 
} else {
set ragent [$node_($i) set ragent_]
$ragent set isBlackhole 1
$ragent set isActive 0
$ragent set isGrayhole 0 
}
}

#Define traffic model
source $opt(cp)

#Define node movement model
source $opt(sc)

#Define node initial position in nam
for {set i 0} {$i < $val(nn)} {incr i} {
#30 defines the node size in nam, must adjust it according to each case
#The function must be called after mobility model is defined
$ns_ initial_node_pos $node_($i) 30
}

#Tell nodes when the simulation ends
for {set i 0} {$i < $val(nn)} {incr i} {
$ns_ at $val(stop) "$node_($i) reset"
}

$ns_ at $val(stop) "$ns_ nam-end-wireless $val(stop)"
$ns_ at $val(stop) "stop"
$ns_ at 900.1 "puts \"NS exiting...­\"; $ns_ halt"
proc stop {} {
global ns_ tracefd namtracefd
$ns_ flush-trace
close $tracefd
close $namtracefd
#exec nam aodv.nam &
exit 0
}
$ns_ run
