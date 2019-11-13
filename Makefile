CFLAGS = -c -O3 -Wall
libs = storage.o virtual_disk.o oufs_lib_support.o oufs_lib.o
EXEC = oufs_inspect oufs_stats oufs_format oufs_ls oufs_mkdir oufs_rmdir oufs_append oufs_cat oufs_copy oufs_create oufs_link oufs_remove oufs_touch
INCLUDES = storage.h oufs_lib_support.h oufs_lib.h virtual_disk.h

all: $(libs) $(EXEC)

#$(EXEC): $(libs) $(INCLUDES)
#	gcc $^.o $< $(libs) -o $@

oufs_inspect: oufs_inspect.o $(libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_stats: oufs_stats.o $(libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_format: oufs_format.o $(libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_ls: oufs_ls.o $(libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_mkdir: oufs_mkdir.o $(libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_rmdir: oufs_rmdir.o $(libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_append: oufs_append.o $(libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_cat: oufs_cat.o $($libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_copy: oufs_copy.o $($libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_create: oufs_create.o $($libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_link: oufs_link.o $(libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_remove: oufs_remove.o $($libs) $(INCLUDES)
	gcc $< $(libs) -o $@

oufs_touch: oufs_touch.o $($libs) $(INCLUDES)
	gcc $< $(libs) -o $@

.c.o:
	gcc $(CFLAGS) $< -o $@

clean:
	rm -f *.o ${EXEC}

zip:
	zip project3.zip README.txt *.c *.h Makefile

