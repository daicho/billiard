TARGET = j16426.exe

SRCS = billiard.c ball.c table.c vector.c shape.c
ICON = icon.ico

ICON_RC = ${ICON:.ico=.rc}
ICON_OBJ = ${ICON:.ico=.o}
OBJS = ${SRCS:.c=.o} $(ICON_OBJ)

HEADERS = billiard.h ball.h table.h vector.h shape.h

CC = gcc
CCFLAGS = -Wall -I/usr/include/opengl
LD = gcc
LDFLAGS =
LIBS = -lm -lglpng -lglut32 -lglu32 -lopengl32

$(TARGET): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(TARGET) $(LIBS)

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CCFLAGS) -c $<

$(OBJS): $(HEADERS) Makefile

$(ICON_OBJ): $(ICON) $(ICON_RC)
	windres -i $(ICON_RC) -o $(ICON_OBJ)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) core *-
