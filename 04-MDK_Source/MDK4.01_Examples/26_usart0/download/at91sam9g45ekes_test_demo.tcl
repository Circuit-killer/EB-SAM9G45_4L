################################################################################
#    SAM-BA history file
################################################################################

################################################################################
    global target
################################################################################

NANDFLASH::Init
GENERIC::EraseAll
GENERIC::SendBootFile "nandflash_at91sam9g45ekes.bin"
send_file {NandFlash} "at91sam9G45.bin" 0x20000 0


