PROJECT_NAME=membrane-media-player
APPLICATION_NAME="Membrane Media Player"
APPLICATION_PACKAGE_NAME=MembraneMediaPlayer
PLATFORM=$(shell uname)
PLATFORM_CFLAGS=
ifeq ($(PLATFORM), Linux)
PLATFORM_ID=linux
PLATFORM_CFLAGS=-DPLATFORM_LINUX=1
CC=g++
endif
ifeq ($(PLATFORM), Darwin)
PLATFORM_ID=macos
PLATFORM_CFLAGS=-DPLATFORM_MACOS=1 -std=c++11
CC=g++
endif

ifndef BUILD_ID
BUILD_ID=0-ffffffff
endif
ifndef PLATFORM_ID
PLATFORM_ID=unknown
endif

CURL_PREFIX?= /usr
FFMPEG_PREFIX?= /usr
FREETYPE_PREFIX?= /usr
JPEG_PREFIX?= /usr
LIBPNG_PREFIX?= /usr
LUA_PREFIX?= /usr
OPENSSL_PREFIX?= /usr
SDL2_PREFIX?= /usr
SDL2IMAGE_PREFIX?= /usr
ZLIB_PREFIX?= /usr
SQLITE3_AMALGAMATION=src/sqlite3.c

SRC_PATH=src
O=\
	ActionWindow.o \
	App.o \
	AppCardWindow.o \
	AppNews.o \
	AppUrl.o \
	AppUtil.o \
	Buffer.o \
	Button.o \
	ButtonGradientEndSprite.o \
	ButtonGradientMiddleSprite.o \
	CaptureWriter.o \
	CardLabelWindow.o \
	CardView.o \
	Chip.o \
	ClassId.o \
	Color.o \
	ComboBox.o \
	ConsoleWindow.o \
	Database.o \
	DoubleList.o \
	FloatList.o \
	Font.o \
	FsBrowserWindow.o \
	HashMap.o \
	HelpActionWindow.o \
	HelpWindow.o \
	HyperlinkWindow.o \
	IconLabelWindow.o \
	Image.o \
	ImageWindow.o \
	Input.o \
	Int64List.o \
	IntList.o \
	Ipv4Address.o \
	json-builder.o \
	json-parser.o \
	Json.o \
	Label.o \
	LabelWindow.o \
	ListView.o \
	Log.o \
	LuaAwaitloadimagesFunction.o \
	LuaAwaitvalueFunction.o \
	LuaCapturescreenFunction.o \
	LuaCapturescreenpathFunction.o \
	LuaClickFunction.o \
	LuaDofileFunction.o \
	LuaFunction.o \
	LuaFunctionList.o \
	LuaGetenvFunction.o \
	LuaHelpFunction.o \
	LuaKeypressFunction.o \
	LuaMenuselectFunction.o \
	LuaMouseleftclickFunction.o \
	LuaMouseoverFunction.o \
	LuaMousepointerFunction.o \
	LuaMousewarpFunction.o \
	LuaOpenFunction.o \
	LuaPrintcontrolsFunction.o \
	LuaPrintFunction.o \
	LuaQuitFunction.o \
	LuaScript.o \
	LuaSetvalueFunction.o \
	LuaSleepFunction.o \
	LuaStartstreamFunction.o \
	LuaStopstreamFunction.o \
	LuaStreampathFunction.o \
	LuaTargetFunction.o \
	LuaTimeoutFunction.o \
	LuaUntargetFunction.o \
	Main.o \
	MainToolbarWindow.o \
	MathUtil.o \
	MediaControl.o \
	MediaControlSearch.o \
	MediaControlWindow.o \
	MediaItem.o \
	MediaItemDetailWindow.o \
	MediaItemImageWindow.o \
	MediaItemTagWindow.o \
	MediaItemUi.o \
	MediaItemWindow.o \
	MediaPlaylist.o \
	MediaPlaylistUi.o \
	MediaPlaylistViewWindow.o \
	MediaPlaylistWindow.o \
	MediaReader.o \
	MediaSearch.o \
	MediaThumbnailWindow.o \
	MediaUtil.o \
	MediaWriter.o \
	Menu.o \
	Network.o \
	OsUtil.o \
	Panel.o \
	PlayerControl.o \
	PlayerTimelineWindow.o \
	PlayerUi.o \
	PlayerWindow.o \
	PlayMarker.o \
	Position.o \
	Prng.o \
	ProgressBar.o \
	ProgressRing.o \
	ProgressRingSprite.o \
	RecordStore.o \
	RenderResource.o \
	Resource.o \
	RoundedCornerSprite.o \
	ScrollBar.o \
	ScrollView.o \
	ScrollViewWindow.o \
	SdlUtil.o \
	SettingsWindow.o \
	SharedBuffer.o \
	Slider.o \
	SliderThumbSprite.o \
	SliderWindow.o \
	SnackbarWindow.o \
	SoundMixer.o \
	SoundSample.o \
	Sprite.o \
	SpriteGroup.o \
	SpriteId.o \
	sqlite3.o \
	StatsWindow.o \
	StdString.o \
	StringList.o \
	SystemInterface.o \
	TagActionWindow.o \
	TaskGroup.o \
	TextArea.o \
	TextCardWindow.o \
	TextField.o \
	TextFieldWindow.o \
	TextFlow.o \
	Toggle.o \
	ToggleWindow.o \
	Toolbar.o \
	TooltipWindow.o \
	Ui.o \
	UiConfiguration.o \
	UiLog.o \
	UiLogWindow.o \
	UiStack.o \
	UiText.o \
	Video.o \
	Widget.o

VPATH=$(SRC_PATH)
CFLAGS=$(PLATFORM_CFLAGS) \
	-I$(CURL_PREFIX)/include \
	-I$(FFMPEG_PREFIX)/include \
	-I$(FREETYPE_PREFIX)/include \
	-I$(FREETYPE_PREFIX)/include/freetype2 \
	-I$(JPEG_PREFIX)/include \
	-I$(LIBPNG_PREFIX)/include \
	-I$(LUA_PREFIX)/include \
	-I$(OPENSSL_PREFIX)/include \
	-I$(SDL2_PREFIX)/include \
	-I$(SDL2_PREFIX)/include/SDL2 \
	-I$(SDL2IMAGE_PREFIX)/include \
	-I$(ZLIB_PREFIX)/include \
	-I$(SRC_PATH) $(EXTRA_CFLAGS)
LDFLAGS=-L$(CURL_PREFIX)/lib \
	-L$(FFMPEG_PREFIX)/lib \
	-L$(FREETYPE_PREFIX)/lib \
	-L$(JPEG_PREFIX)/lib \
	-L$(LIBPNG_PREFIX)/lib \
	-L$(LUA_PREFIX)/lib \
	-L$(OPENSSL_PREFIX)/lib \
	-L$(SDL2_PREFIX)/lib \
	-L$(SDL2IMAGE_PREFIX)/lib \
	-L$(ZLIB_PREFIX)/lib \
	$(EXTRA_LDFLAGS)
LD_STATIC_LIBS=$(LUA_PREFIX)/lib/liblua.a
LD_DYNAMIC_LIBS=-lSDL2 \
	-lSDL2_image \
	-lfreetype \
	-lavcodec -lavutil -lavdevice -lavfilter -lavformat -lpostproc -lswresample -lswscale -lx264 \
	-ldl \
	-lm \
	-lpthread \
	-lcurl \
	-lssl \
	-lcrypto \
	-lpng \
	-ljpeg \
	-lz

all: $(PROJECT_NAME)

clean:
	rm -f $(O) $(PROJECT_NAME) $(SRC_PATH)/BuildConfig.h

$(SRC_PATH)/BuildConfig.h:
	@echo "#ifndef BUILD_CONFIG_H" > $@
	@echo "#define BUILD_CONFIG_H" >> $@
	@echo "#ifndef BUILD_ID" >> $@
	@echo "#define BUILD_ID \"$(BUILD_ID)\"" >> $@
	@echo "#endif" >> $@
	@echo "#ifndef PLATFORM_ID" >> $@
	@echo "#define PLATFORM_ID \"$(PLATFORM_ID)\"" >> $@
	@echo "#endif" >> $@
	@echo "#ifndef APPLICATION_NAME" >> $@
	@echo "#define APPLICATION_NAME \"$(APPLICATION_NAME)\"" >> $@
	@echo "#endif" >> $@
	@echo "#ifndef APPLICATION_PACKAGE_NAME" >> $@
	@echo "#define APPLICATION_PACKAGE_NAME \"$(APPLICATION_PACKAGE_NAME)\"" >> $@
	@echo "#endif" >> $@
	@echo "#endif" >> $@

$(PROJECT_NAME): $(SRC_PATH)/BuildConfig.h $(O)
	$(CC) -o $@ $(O) $(LD_STATIC_LIBS) $(LDFLAGS) $(LD_DYNAMIC_LIBS)

.SECONDARY: $(O)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

json-parser.o: json-parser.c
	$(CC) -o $@ -g -c $<

json-builder.o: json-builder.c
	$(CC) -o $@ -g -c $<

sqlite3.o: $(SQLITE3_AMALGAMATION)
	$(CC) -o $@ -g -x c -c $<
