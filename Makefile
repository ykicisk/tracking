COMPILER = clang++
CFLAGS   = -std=c++11 -ftemplate-depth=512 -Wall -MMD -MP -O3 #-g #-fopenmp
LDFLAGS  = $(shell pkg-config --libs opencv) -L/usr/local/lib -lboost_system -lboost_filesystem 
LIBS     = 
INCLUDE  = $(shell pkg-config --cflags opencv) #-Iinclude $(shell pkg-config --cflags opencv eigen3)
TARGETTEST = test
TARGET = lt_test
OBJDIR   = ./obj
ifeq "$(strip $(OBJDIR))" ""
	OBJDIR = .
endif
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SOURCES  = $(call rwildcard , src, *.cpp)
OBJECTS  = $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))
DEPENDS  = $(OBJECTS:.o=.d)

$(TARGET): $(OBJECTS) $(LIBS)
	$(COMPILER) -o $@ $^ $(LDFLAGS)
#	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
#	ar r $@ $^

$(OBJDIR)/%.o: %.cpp
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(COMPILER) $(CFLAGS) $(INCLUDE) -o $@ -c $<

all : $(TARGET) $(TARGETTEST)

$(TARGETTEST): $(TARGET) $(OBJDIR)/test.o
	$(COMPILER) $(CXXFLAGS) -o $@ $(OBJDIR)/test.o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
	@rmdir --ignore-fail-on-non-empty $(OBJDIR)

-include $(DEPENDS)
