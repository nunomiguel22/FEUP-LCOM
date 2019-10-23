PROG=proj

SRCS = proj.c mvector.c vcard.c devices.c ship.c asteroidfield.c gamestate.c graphics.c multiplayer.c serialport.c 


CPPFLAGS += -pedantic -D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf -lm

.include <minix.lcom.mk>
