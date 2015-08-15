define memdump
	dump memory dump-ccsram.bin 0x10000000 0x10010000
	dump memory dump-sram.bin   0x20000000 0x20030000
	shell x=$(date +%Y%m%d%-H%M%S); mkdir memdump-$x; mv dump*.bin memdump-$x
end
