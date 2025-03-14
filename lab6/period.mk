SHELL=cmd
CC = xc32-gcc
OBJCPY = xc32-bin2hex
ARCH = -mprocessor=32MX130F064B
OBJ = Period.o
PORTN=$(shell type COMPORT.inc)

Period.elf: $(OBJ)
	$(CC) $(ARCH) -o Period.elf Period.o -mips16 -DXPRJ_default=default -legacy-libc -Wl,-Map=Period.map
	$(OBJCPY) Period.elf
	@echo Success!
   
Period.o: Period.c
	$(CC) -mips16 -g -x c -c $(ARCH) -MMD -o Period.o Period.c -DXPRJ_default=default -legacy-libc

clean:
	@del *.o *.elf *.hex *.map *.d 2>NUL
	
LoadFlash:
	@taskkill /f /im putty.exe /t /fi "status eq running" > NUL
	pro32 -p Period.hex
	cmd /c start putty.exe -serial $(PORTN) -sercfg 115200,8,n,1,N

putty:
	@taskkill /f /im putty.exe /t /fi "status eq running" > NUL
	cmd /c start putty.exe -serial $(PORTN) -sercfg 115200,8,n,1,N

dummy: Period.hex Period.map
	$(CC) --version