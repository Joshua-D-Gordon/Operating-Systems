SUBDIRS = server client

all:
	@for dir in $(SUBDIRS); do \
		echo "Making all in $$dir"; \
		$(MAKE) -C $$dir all; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done