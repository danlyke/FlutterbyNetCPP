#WEBSITEBASE=/home/danlyke/websites/flutterby.net/code
WEBSITEBASE=$(shell (test -d /home/danlyke/websites/flutterby.net/code && echo /home/danlyke/websites/flutterby.net/code) || (test -d /home/danlyke/websites/flutterby.net/ && echo /home/danlyke/websites/flutterby.net/) )

HTMLSTAGING=/home/danlyke/var/flutterby_net_staging/
#HTMLTARGET=/var/www/
HTMLTARGET=/home/danlyke/websites/flutterby.net/public_html_static/

vpath %.cpp .

CC=g++
CFLAGS= -std=c++11 -Wall -Wextra -Weffc++ -g -fPIC -I /usr/include/postgresql
LDFLAGS= -L. -lfby -lsqlite3 -lpq -lboost_filesystem -lboost_system -lgdbm -lpcre -lboost_iostreams  -l boost_program_options
LIBSOURCES=fbystd.cpp parsertokens.cpp textparse.yy.cpp stringutil.cpp parsetree.cpp fbyregex.cpp fby_core_unix_array.cpp fby_core_unix_helpers.cpp fby_core_unix.cpp fbydb.cpp fbydb_sqlite3.cpp fbydb_postgresql.cpp regexparser.cpp treebuilder.cpp wikistrings.cpp

SOURCES=sqldefinitions.cpp wiki.cpp wikidb.cpp wikiobjects.cpp wikinode.cpp wikivideoflash.cpp wikistatusupdate.cpp  wikiopenlayers.cpp wikidpl.cpp openlayerstemplate.cpp

EXESOURCES=test_file.cpp test_lambda.cpp test_flex.cpp test_tester.cpp test_reset.cpp test_regex.cpp test_gdbm.cpp test_removecrs.cpp test_readstdout.cpp test_sqlite.cpp sqlextractor.cpp test_strftime.cpp test_findimageinfo.cpp  test_iter.cpp test_wikistring.cpp  fixedstringextractor.cpp test_boost_options.cpp
# copychangedfiles.cpp
# test_subst.cpp

EXE2SOURCES= fby.cpp test_fbydb.cpp scaninputwikifiles.cpp scanimages.cpp parsewikifiletostaging.cpp test_regexparse.cpp parsewikifilesforlinks.cpp scanwikifilesforlinks.cpp rebuilddirtyfiles.cpp 


OUTPUTDIRECTORIES= ../obj ../bin

LIBOBJECTS=$(patsubst %.cpp,../obj/%.o,$(LIBSOURCES))
OBJECTS=$(patsubst %.cpp,../obj/%.o,$(SOURCES))
EXEOBJECTS=$(patsubst %.cpp,../obj/%.o,$(EXESOURCES))
EXECUTABLE=$(patsubst %.cpp,../bin/%,$(EXESOURCES))

EXE2OBJECTS=$(patsubst %.cpp,../obj/%.o,$(EXE2SOURCES))
EXECUTABLE2=$(patsubst %.cpp,../bin/%,$(EXE2SOURCES))

all: $(EXECUTABLE) $(EXECUTABLE2) $(OUTPUTDIRECTORIES) ../var/fby.sqlite3
	mkdir -p $(OUTPUTDIRECTORIES)
	../bin/test_boost_options


#	mkdir -p $(HTMLSTAGING)
#	../bin/wikistringtest
#	../bin/scaninputwikifiles $(WEBSITEBASE)/mvs $(WEBSITEBASE)/html_staging/
#	../bin/parsewikifilesforlinks $(WEBSITEBASE)'/mvs/Visiting the Bay Area.wiki'
#	../bin/scanwikifilesforlinks $(WEBSITEBASE)'/mvs/' $(WEBSITEBASE)/html_staging/
#	../bin/rebuilddirtyfiles $(HTMLTARGET)
#	../bin/parsewikifiletostaging $(WEBSITEBASE)/mvs/Hiking*.wiki $(HTMLTARGET)
#	../bin/parsewikifiletostaging $(WEBSITEBASE)/mvs/*.wiki ../html_staging/
#	/usr/bin/time ../bin/scanimages /var/www/images/ /home/danlyke/websites/flutterby.net/public_html_static/

website:
	rm ../var/fby.sqlite3
	sqlite3 ../var/fby.sqlite3 < sqldefinitions.sql
	sqlite3 ../var/fby.sqlite3 < sqltestdata.sql 
	/usr/bin/time ../bin/scanimages $(HTMLTARGET)
	../bin/scanwikifilesforlinks $(WEBSITEBASE)'/mvs/' $(HTMLTARGET)
	../bin/parsewikifiletostaging $(WEBSITEBASE)/mvs/*.wiki $(HTMLTARGET)

scanimages:
	/usr/bin/time ../bin/scanimages $(HTMLTARGET)


database:
	-rm ../var/fby.sqlite3
	sqlite3 ../var/fby.sqlite3 < sqldefinitions.sql
	sqlite3 ../var/fby.sqlite3 < sqltestdata.sql 
	/usr/bin/time ../bin/scanimages /var/www/images/ /home/danlyke/websites/flutterby.net/public_html_static/

openlayerstemplate.cpp : openlayerstemplate.js ../bin/fixedstringextractor
	../bin/fixedstringextractor sections openlayerstemplate.js openlayerstemplate

../var/fby.sqlite3 : sqldefinitions.sql
	mkdir -p ../var
	-rm ../var/fby.sqlite3
	sqlite3 ../var/fby.sqlite3 < sqldefinitions.sql
	sqlite3 ../var/fby.sqlite3 < sqltestdata.sql 


#	../bin/fbydbtest
#	./test_sqlite 'CREATE abc (id INTEGER PRIMARY KEY AUTOINCREMENT, t TEXT)'
#	./regexparse t/markdown.txt
#	./test_removecrs
#	./parser < t/markdown.txt
#	./flextest < t/markdown.txt

sqldefinitions.cpp : wikiobjects.h ../bin/sqlextractor
	../bin/sqlextractor wikiobjects.h

$(OBJECTS): | obj
$(LIBOBJECTS): | obj
$(EXEOBJECTS): | obj
$(EXE2OBJECTS): | obj

../obj/%.o : %.cpp

obj :
	@mkdir -p $@

../obj/%.o : %.cpp
	$(CC) -c $(CFLAGS) $< -o $@
	$(CC) -MM $(CFLAGS) $< > ../obj/../obj/$*.d
	@mv -f ../obj/$*.d ../obj/$*.d.tmp
	@sed -e 's|.*:|$*.o:|' < ../obj/$*.d.tmp > ../obj/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < ../obj/$*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> ../obj/$*.d
	@rm -f ../obj/$*.d.tmp



clean:
	-rm -f $(OBJECTS) $(LIBOBJECTS) $(EXECUTABLE) $(EXEOBJECTS) \
           $(EXE2OBJECTS) $(EXECUTABLE2) \
           libfby.a \
           sqldefinitions.cpp \
	       parsertokens.cpp parsertokens.h textparse.yy.cpp textparse.yy.h \
	       *.d t/*.tmp


test_flex.cpp : parsertokens.cpp

$(EXECUTABLE2): $(EXE2OBJECTS) $(OBJECTS) libfby.a
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(patsubst ../bin/%,../obj/%.o,$@) $(LDFLAGS)

$(EXECUTABLE): $(EXEOBJECTS) libfby.a
	$(CC) $(CFLAGS) -o $@ $(patsubst ../bin/%,../obj/%.o,$@) $(LDFLAGS)

libfby.a : $(LIBOBJECTS)
	ar rvs $@ $(LIBOBJECTS)


# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)
-include $(LIBOBJECTS:.o=.d)
-include $(EXEOBJECTS:.o=.d)
-include $(EXE2OBJECTS:.o=.d)

%.o : %.cpp
	$(CC) -c $(CFLAGS) $*.cpp -o $*.o
	$(CC) -MM $(CFLAGS) $*.cpp > ../obj/$*.d
	@mv -f ../obj/$*.d ../obj/$*.d.tmp
	@sed -e 's|.*:|$*.o:|' < ../obj/$*.d.tmp > ../obj/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < ../obj/$*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> ../obj/$*.d
	@rm -f ../obj/$*.d.tmp



parsertokens.cpp : parsertokens.txt createparsertokens.pl
	./createparsertokens.pl

textparse.yy.cpp : textparse.l
	flex++ --c++ --outfile textparse.yy.cpp --yyclass=TextParse textparse.l

