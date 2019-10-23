#include <machine/asm.h>
#include "i8254.h"


.global _C_LABEL (kbc_asm_ih)
.global _C_LABEL (code)
.data


.text
_C_LABEL (kbc_asm_ih) :


	err:
	INb $0x64, %al  #Move status byte to AL
	movb $0xC0, %bl  #Parity and Timeout erros
	and %al, %bl #check if status has errors
	jnz err #repeat while errors found
	movl $0x00000000, %eax #reset eax
	INb $0x60, %al #Move OUT_BUF to al
	movl %eax, _C_LABEL(code)  #Move OUT_BUF to global variable code
	
	
	
