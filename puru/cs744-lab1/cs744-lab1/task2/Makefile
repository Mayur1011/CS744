.PHONY: all clean dir1 dir2

SUBDIRS := helloworld hellouniverse

all: dir1 dir2

dir1:
	$(MAKE) -C helloworld

dir2:
	$(MAKE) -C hellouniverse

clean:
	$(MAKE) -C helloworld clean
	$(MAKE) -C hellouniverse clean
